# Web-registration-system

In this project, a simple web registration system for USC is implemented. Specifically, a
student will use the client to access the central web registration server, which will forward their
requests to the department servers in each department. For each department, the department
server will store the information of the courses offered in this department. Additionally, a
credential server will be used to verify the identity of the student.
There are total 5 communication end-points:
● Client: used by a student to access the registration system.
● Main server (serverM): coordinate with the backend servers.
● Credential server (serverC): verify the identity of the student.
● Department server(s) (serverCS and serverEE)): store the information of courses offered
by this department.

For the backend servers, Credential server and Department servers will access corresponding
files on disk, and respond to the request from the main server based on the file content. It is
important to note that only the corresponding server should access the file. It is prohibited to
access a file in the main server or other servers. We will use both TCP and UDP connections.
However, we assume that all the UDP packages will be received without any error.

## Source Code Files
1. ServerM (Main Server): You must name your code file: serverM.c or serverM.cc
or serverM.cpp (all small letters except ‘M’). Also you must include the
corresponding header file (if you have one; it is not mandatory) serverM.h (all
small letters except ‘M’).
2. Backend-Servers C, CS and EE: You must use one of these names for this piece of
code: server#.c or server#.cc or server#.cpp (all small letters except for #). Also
you must include the corresponding header file (if you have one; it is not
mandatory). server#.h (all small letters, except for #). The “#” character must be
replaced by the server identifier (i.e. C or CS or EE), depending on the server it
corresponds to. (e.g., serverC.cpp, serverEE.cpp & serverCS.cpp)
Note: You are not allowed to use one executable for all four servers (i.e. a “fork”
based implementation).
3. Client: The name of this piece of code must be client.c or client.cc or client.cpp
(all small letters) and the header file (if you have one; it is not mandatory) must be
called client.h (all small letters).

## Input Files:
There are three input files that are given to the credential Server and two department
servers and should be read by the server when it is up and running.
● cred.txt: contains encrypted usernames and passwords. This file should only be
accessed by the Credential server.
● ee.txt: contains course information categorized in course code, credit, professor,
days and course name. Different categories are separated by a comma. There could
be space(s) or semicolons, except commas, in a category. One example is given
below. This file should only be accessed by the EE Department server.
EE450,4,Ali Zahid,Tue;Thu,Introduction to Computer Networks
● cs.txt: Same format as ee.txt. This file should only be accessed by the CS
Department server.

## Reused Code:
https://beej.us/guide/bgnet/html/split/index.html

I have reused the code from "Beej's guide to socket programming", and functions include getting the address information of a host, binding the host to a port, connecting the hosts, accepting the connection, sending and receiving the messages.
In detail, these functions are getaddrin(), socket(), bind(), connect(), listen(), accept(), send()/sendto(), recv()/recvfrom().
