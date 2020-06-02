# AssignmentCAPS - Gergo Kekesi

A TCP client server application using the winsock library, designed to achieve high throughput.

- AssignmentCAPS
	- TCP_Client
		- client_main.cpp: The main file of the TCP Client containing the code for the 
				   test harness which spawns x number of given threads running them for
				   n number of seconds. The thread types and count can be configured.
		- tcp_client.h:	 The header file for the tcp_client provided for the assignment
		- tcp_client.cpp: The implementation source file for the tcp client. Version 2.
	- TCP_Server
		- request_parser.cpp: Implementation of the request parser class which contains
				      classes for the four different request types accepted by the server
				      (POST, READ, COUNT and LIST)
		- request_parser.h: Header file describing the four request classes.
		- server_main.cpp: The main file for the project containing the code that listens for client
				      connections and processes the requests sent by them using a multithreaded
				      architecture.
		- tcp_data.h: The ReceivedSocketData class used by the server to manage the data communication
				      over a TCP socket.
		- tcp_server.cpp: The TCP Server implementation provided again by the Sergio in the Libraries v2.
		- tcp_server.h: The header file describing the implementation for the TCP Server.
		- ThreadPool.h: C++11 implementation of a thread pool retrieved from:
				      https://github.com/progschj/ThreadPool
	- AssignmentCAPS.sln:The Visual Studio 17 solution file.

Compiling Instructions:
- The project was built with Visual Studio 2017 therefore to compile just open the AssignmentCAPS.sln file.
 Ensure compilation is done in Release mode for an x64 architecture. 
- The solution contains two projects, TCP_Client and TCP_Server. 
- The solution is configured to build both but if that's not the case, they can be built individually too.

Running of the test harness:
- In order to run the test harness, it must have been compiled and built first.
- The output should be in the root of this folder under x64/Release.
- Firstly, ensure that the TCP_Server is running.
- Then, launch a new command line in the directory of the TCP_Client build folder
- Start the client by specifying the IP address as localhost (127.0.0.1) with the following command: TCP_Client.exe 127.0.0.1
- You will now be asked to specify the number of clients/threads you'd like to create for each request type.
- For the scope of this assignment, only the poster and reader threads were varied.
- There is no validation of input so ensure that you only type nubmers in.
- To accept a number press enter.
- After specifying the number of threads for each request type, please specify the time in seconds you'd like the clients to send messages for
- Press ENTER and leave the client to do it's work. The computer may be a little unresponsive during testing.
- To not affect the result, the computer should be left untouched until the given time is over.
- The results will be presented once the 10 seconds are elapsed as a list of individual throughputs per thread as well as the average.
- Pressing any key again will terminate the program.

PLEASE NOTE:
- The server/server do not successfuly close the connection as I was unable to fix this issue before the deadline.
- This means the server should be restarted after each run (which is the case anyway as specified in the marking scheme).
- The throughput evaluations were ran with most of the software closed on the computer so as to not affect the processing speed of the computer
- Each test was ran 10 times and the average was taken to acquire the results.

