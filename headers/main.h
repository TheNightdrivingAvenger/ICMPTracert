#define DEFAULT_PACKET_SIZE 40
#define MAX_PING_PACKET_SIZE 1024

#define DEFAULT_HOPS 30
#define DEFAULT_PACK_COUNT 3
#define DEFAULT_TIME_OUT 3000
#define DEFAULT_RESOLVING_MODE 1

#define FLAGS_COUNT 4

#define IP_SOURCE_ADDR_OFFSET 12

#define ICMP_ECHO_REPLY 0
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

#define TRACE_TTL_EXP 1
#define TRACE_END_SUCCESS 2
#define WRONG_PACKET -1

#define IPHDR_LEN_MASK 0x0F

#pragma pack(push,1)
typedef struct _tag_IPHeader {
	BYTE ver_n_len;
	BYTE srv_type;
	USHORT total_len;
	USHORT pack_id;
	USHORT flags:3;
	USHORT offset:13;
	BYTE TTL;
	BYTE proto;
	USHORT checksum;
	UINT source_ip;
	UINT dest_ip;
} IPHeader, *PIPHeader;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _tag_ICMPHeader {
	BYTE msg_type;
	BYTE msg_code;
	USHORT checksum;
	USHORT id;
	USHORT seq;
} ICMPHeader, *PICMPHeader;
#pragma pack(pop)

typedef struct _tag_Arguments {
	int hopsCount;
	int packetCount;
	DWORD timeOut;
	BOOL resolveName;
} Arguments, *PArguments;

typedef struct _tag_PacketDetails {
	struct sockaddr_in *source;
	DWORD ping;
} PacketDetails, *PPacketDetails;
