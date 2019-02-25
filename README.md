# ICMP Tracert

## About
This is an implementation of Windows's tracert program that uses (just like the original) ICMP packets, written in raw C + WinAPI (and a bit of C standard library for easier output).
This program uses non-unicode WinAPI functions, so **using unicode domain names won't work** (also problems can show up if your PC has unicode characters in its name or if the name is longer than 50 characters).
Only *IPv4* is supported (both your PC and target node must support IPv4).

## How to build
*Developed with Pelles C for Windows v9.00.*

To build and run:
1. Create a Win32 Console project in Pelles C.
2. Go to project -> project options -> compiler and enable Microsoft extensions.
3. Go to file -> new -> source code, copy main.c contents there, then save it in the project's folder (choose "Source file (.c)" in the file type field; agree with adding the file to the current project). Create another source code, copy main.h contents there, then create folder "headers" in the project directory, save the file in this "headers" folder with the name "main" (choose "Include file (.h)" in the file type field).
4. Because of using RAW sockets, in case of any issues you should try running Pelles C (or compiled program) with Administrator rights.

Target IP/domain name is passed through command line arguments (as well as some flags). To add them, go to project -> project options -> general.

5. Any other compiler and builder should be fine too, just make sure it supports Microsoft SDK.

## How to use
Supported flags for setting: timeout, sent packet count, maximum hops, and resolve/do not resolve IPs to domain names mode. To see full help launch the program without arguments.

Launch the program passing flags (not necessary) and destination node (IP or domain name, necessary) as command-line arguments.
