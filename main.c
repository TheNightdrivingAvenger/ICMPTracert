﻿#define __STDC_WANT_LIB_EXT1__ 1

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <string.h>

#include <stdio.h>
#include "headers\main.h"

/*--------------------------------
******TO DO*******
2. Use non-deprecated functions
3. Find out what's this strange +4 bytes in ICMP time-out header
4. Add name resolving and IP-to-name
*********************************/


const char *const USING_GUIDE = "How to use: traceroute [flags] <destination IP>\nflags:\n-h <max_hops>\t\t define the maximum hops count (>= 1)\n\
-n <pack_num>\t\t define how many packetes (>= 1) to send to each node\n-w <time_to_wait>\t define time-out for each packet in milliseconds\n";

const char *const STARTUP_MSG = "Tracing route to ";
const char *const STARTUP_MSG2 = ", sending ";
const char *const STARTUP_MSG3 = " packets of size 32 to each router with maximum of ";
const char *const STARTUP_MSG4 = " hops\n\n";
const char *const INVALID_ARGS = "Invalid params\n";
const char *const SENDING_FAILED = "Failed to send data, exiting...\n";
const char *const TIME_OUT = "   T/O";
const char *const SUCCESS_END = "Tracing completed\n";
const char *const NOT_ENOUGH_HOPS = "Hops limit depleted, destination was not reached\n";
const char *const FAIL_END = "traceroute ended with failure\n";

const char *const DLL_FAILED = "DLL failed to load!\n";
const char *const SCKT_FAILED = "Failed to create RAW-socket\n";
const char *const ADDR_NOTFOUND = "Appropriate address for connection not found\n";
const char *const ARGS_ERR = "Error in args\n";
const char *const RES_FAILED = "Bad IP\n";
const char *const CANNOT_RECIEVE = "Critical error when tried to receive data! Exiting...\n";
const char *const SELECT_FAILED = "Select() failed! Exiting...\n";

void cleanUp(const char *msg)
{
	printf(msg);
	WSACleanup();
	getchar();
}

void invalidArgs(void)
{
	printf(INVALID_ARGS);
	printf(USING_GUIDE);
	getchar();
}

USHORT calcICMPChecksum(USHORT *packet, int size)
{
	ULONG checksum = 0;
	while (size > 1) {
		checksum += *(packet++);
		size -= sizeof(USHORT);
	}
	if (size) checksum += *(UCHAR *)packet; // MAYBE OBSOLETE!

	checksum = (checksum >> 16) + (checksum & 0xFFFF);
	checksum += (checksum >> 16);

	return (USHORT)(~checksum);
}

void initPingPacket(PICMPHeader icmp_hdr, int seqNo)
{
	icmp_hdr->msg_type = ICMP_ECHO_REQUEST;
	icmp_hdr->msg_code = 0;
	icmp_hdr->checksum = 0;
	icmp_hdr->id = LOWORD(GetCurrentProcessId());
	icmp_hdr->seq = seqNo;

	int bytesLeft = DEFAULT_PACKET_SIZE - sizeof(ICMPHeader);
	char *newData = (char *)icmp_hdr + sizeof(ICMPHeader);
	char symb = 'a';
	while (bytesLeft > 0) {
		*(newData++) = symb++;
		bytesLeft--;
	}
	icmp_hdr->checksum = calcICMPChecksum((USHORT *)icmp_hdr, DEFAULT_PACKET_SIZE);
}

int sendPingReq(SOCKET traceSckt, PICMPHeader sendBuf, const struct sockaddr_in *dest)
{
	int sendRes = sendto(traceSckt, (char *)sendBuf, DEFAULT_PACKET_SIZE, 0, (struct sockaddr *)dest, sizeof(struct sockaddr_in));

	if (sendRes == SOCKET_ERROR) return sendRes;
	return 0;
}

int recvPingResp(SOCKET traceSckt, PIPHeader recvBuf, struct sockaddr_in *source, long timeout)
{
	int srcLen = sizeof(struct sockaddr_in);

	fd_set singleSocket;
	singleSocket.fd_count = 1;
	singleSocket.fd_array[0] = traceSckt;

	long microseconds = timeout * 1000;

	struct timeval timeToWait = {microseconds / 1000000, microseconds % 1000000};

	int selectRes;
	if ((selectRes = select(0, &singleSocket, NULL, NULL, &timeToWait)) == 0) return 0; // time-out
	if (selectRes == SOCKET_ERROR) return 1;
	
	return recvfrom(traceSckt, (char *)recvBuf, MAX_PING_PACKET_SIZE, 0, (struct sockaddr *)source, &srcLen);
}

void printPackInfo(struct sockaddr_in *source, DWORD ping, BOOL printIP)
{
	printf("%6d", ping);

	if (printIP) {
		char *srcAddr = inet_ntoa(source->sin_addr);
		if (srcAddr != NULL) {
			printf("		%s", srcAddr);
		} else {
			printf("		INVALID IP");
		}
	}
}

int decodeReply(PIPHeader ipHdr, struct sockaddr_in *source, USHORT seqNo, BOOL printIP, ULONG sendingTime)
{
	DWORD arrivalTime = GetTickCount();

	unsigned short ipHdrLen = (ipHdr->ver_n_len & IPHDR_LEN_MASK) * 4;
	PICMPHeader icmpHdr = (PICMPHeader)((char *)ipHdr + ipHdrLen);

	if (icmpHdr->msg_type == ICMP_TTL_EXPIRE) {
		PIPHeader requestIPHdr = (PIPHeader)((char *)icmpHdr + 4 + 4); // strange +4 bytes
		unsigned short requestIPHdrLen = (requestIPHdr->ver_n_len & IPHDR_LEN_MASK) * 4;

		PICMPHeader requestICMPHdr = (PICMPHeader)((char *)requestIPHdr + requestIPHdrLen);

		if ((requestICMPHdr->id == GetCurrentProcessId()) && (requestICMPHdr->seq == seqNo)) {
			printPackInfo(source, arrivalTime - sendingTime, printIP);
			return TRACE_TTL_EXP;
		}
	}

	if (icmpHdr->msg_type == ICMP_ECHO_REPLY) {
		if ((icmpHdr->id == GetCurrentProcessId()) && (icmpHdr->seq == seqNo)) {
			printPackInfo(source, arrivalTime - sendingTime, printIP);
			return TRACE_END_SUCCESS;
		}
	}

	return WRONG_PACKET;
}

int recvAndDecode(SOCKET traceSckt, PIPHeader recvBuf, struct sockaddr_in *source, long timeout, USHORT seqNo, BOOL printIP, ULONG sendingTime)
{
	int recvRes = recvPingResp(traceSckt, recvBuf, source, timeout);
	if (recvRes == 0) printf(TIME_OUT);
	else if (recvRes == 1) printf(SELECT_FAILED);
	else if (recvRes == SOCKET_ERROR) printf(CANNOT_RECIEVE);
	else {
		return decodeReply(recvBuf, source, seqNo, printIP, sendingTime);
	}
	return (recvRes == 0 ? 0 : 3); // if time-out return 0, else return something that won't interfere with decodeReply's return value
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		invalidArgs();
		return 0;
	}

	// reading flags and their values; most probably not a very fast and optimal way
	char *flags[FLAGS_COUNT] = {"-h", "-n", "-w"};
	int maxHops = DEFAULT_HOPS, packCount = DEFAULT_PACK_COUNT;
	long timeout = DEFAULT_TIME_OUT;
	for (int i = 1; i < argc - 1; i++) {
		int j = 0;
		for (; j < FLAGS_COUNT; j++) {
			if (strcmp(argv[i], flags[j]) == 0) {
				if (i + 1 == argc - 1) {
					invalidArgs();
					return 0;
				}
				break;
			}
		}
		if (j == 0) {
			if ((maxHops = atoi(argv[i + 1])) == 0) {
				invalidArgs();
				return 0;
			}
			i++;
		} else if (j == 1) {
			if ((packCount = atoi(argv[i + 1])) == 0) {
				invalidArgs();
				return 0;
			}
			i++;
		} else if (j == 2) {
			if ((timeout = atoi(argv[i + 1])) == 0) {
				invalidArgs();
				return 0;
			}
			i++;
		}
	}

	UINT destAddr = inet_addr(argv[argc - 1]); // getting the IP-addr from cmd params
	if (destAddr == INADDR_NONE) {
		invalidArgs();
		return 0;
	}

	printf(STARTUP_MSG);
	printf(argv[argc - 1]);
	printf(STARTUP_MSG2);
	printf("%d", packCount);
	printf(STARTUP_MSG3);
	printf("%d", maxHops);
	printf(STARTUP_MSG4);

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		cleanUp(DLL_FAILED);
		return 1;
	}

	struct sockaddr_in dest, source;
	SOCKET traceSckt = WSASocket(AF_UNSPEC, SOCK_RAW, IPPROTO_ICMP, NULL, 0, 0);

	if (traceSckt == INVALID_SOCKET) {
		cleanUp(SCKT_FAILED);
		return 2;
	}

	struct sockaddr_in src;

	src.sin_family = AF_INET;

	struct hostent *host_addr_list = gethostbyname(NULL); // deprecated function
	if(host_addr_list == NULL)
	{
		return 3;
	}
	else
	{
	 	src.sin_addr.S_un.S_addr = *((u_long *)(host_addr_list->h_addr_list[0]));
	}

	if (bind(traceSckt, (struct sockaddr *)&src, sizeof(src)) != NO_ERROR) {
		return 4;
	}

	ZeroMemory(&dest, sizeof(dest));
	dest.sin_addr.s_addr = destAddr;
	dest.sin_family = AF_INET;

	PICMPHeader sendBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DEFAULT_PACKET_SIZE);
	PIPHeader recvBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PING_PACKET_SIZE);

	int routerNo = 1;
	USHORT seqNo = 10;	// unsigned overflowing is OK
	ULONG sendingTime;

	BOOL traceEnd = FALSE, error = FALSE, printIP;

	DWORD packageTTL = 0;
	int hops = 0;

	do {
		hops++;
		packageTTL++;
		setsockopt(traceSckt, IPPROTO_IP, IP_TTL, (char *)&packageTTL, sizeof(DWORD));

		printIP = FALSE;
		printf("%3d.", routerNo++);
		for (int packNo = 1; packNo <= packCount; packNo++) {
			if (packNo == packCount) printIP = TRUE;

			initPingPacket(sendBuf, seqNo);
			sendingTime = GetTickCount();
			if (sendPingReq(traceSckt, sendBuf, &dest) == SOCKET_ERROR) {
				printf(SENDING_FAILED);
				error = TRUE;
				break;
			}
			
			int recvRes = recvAndDecode(traceSckt, recvBuf, &source, timeout, seqNo, printIP, sendingTime);
			if (recvRes == 3) {
				error = TRUE;
				break;
			}
			// do I still get all ICMP packets here? Idk. Seems like I don't, so disable "wrong packetes" counter for now
			//int wrongCount = 0;
			// if we get 10 wrong packets, our is probably lost somewhere; bad practise actually
			//while ((recvRes == WRONG_PACKET) && (wrongCount++ < 10)) recvRes = recvAndDecode(traceSckt, recvBuf, &source, timeout, seqNo, printIP);
			//if (recvRes == WRONG_PACKET) printf(TIME_OUT);
			/*else*/ if (recvRes == TRACE_END_SUCCESS) traceEnd = TRUE;
			seqNo++;	
		}
		printf("\n");
	} while (!traceEnd && !error && hops < maxHops);
	
	WSACleanup();
	if (traceEnd) printf(SUCCESS_END);
	else if (!error && !traceEnd) printf(NOT_ENOUGH_HOPS);
	else if (error) printf(FAIL_END);
	getchar();
	return 0;
}
