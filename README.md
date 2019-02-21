# ICMPTracert

Builded and compiled with Pelles C for Windows v9.00.

To build and run:
1. Create a Win32 Console project in Pelles C
2. Go to project -> project options -> compiler and enable Microsoft extensions
3. Go to file -> new -> source code, copy main.c contents there, then save it in the project's folder (choose "Source file (.c)" in the file type field; agree with adding the file to the current project). Create another source code, copy main.h contents there, then create folder "headers" in the project directory, save the file in this "headers" folder with the name "main" (choose "Include file (.h)" in the file type field).
4. Because of using RAW sockets, Pelles C should be run with Administrator rights.

Target IP is passed through command line arguments. To add them, go to project -> project options -> general

# How to use
Supported flags for setting: timeout, sent packet count, maximum hops, and resolve/do not resolve IPs to domain names mode. To see full help launch the program without arguments.

Launch the program passing flags (not necessary) and destination node (IP or domain name, necessary) as command-line arguments.
