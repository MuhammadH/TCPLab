# TCPLab
Simple TCP server/ client code. This was a homework assignment that was kinda fun to finish. I want to keep it here if I ever need to use it as an example for myself in the future. 

Step 1: run makefile in tcpserver
Step 2: run makefile in tcpclient

This will make server (run with ./server) and client (run with ./client IP_address)

Step 3: run both executables and run a bunch of the commands in the client(s)

rand10.txt was used for testing. You can use it for testing, too if you'd like. 


Functions: 
catalog - get a list of files on the server. 
download <source file name> <destination filename> - download a file
upload <source file name> <destination filename>
spwd - get the linux pwd command from the server
bye - disconnect from server


This is just a homework assignment and not meant for actual use, so issues likely won't get fixed. 

Known issues: 
Uploads/ downloads limited to buffer size. 
Control flow not as sweet and simple and compartmentalized as it should be. 