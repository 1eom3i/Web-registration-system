a.Full Name:
Xiaohan Mei

b.ID:
4109123678

c.What have been done?
In this assignment, a web registration which contains 5 ends has been implemented. In detail, the user at the client will first authenticate his account and then if authenticated, he can query the course information from the remote server. 
Optional Part: a multiple course request can be recognized and return its result.

d. Code files:
In Client.cpp, the user will first be allowed to enter their username and password and if correct, they will be authenticated. If not, they should enter again and totally three attempts are allowed. If authenticated, users can query multiple courses infomation or specific category of one course.

In serverC.cpp, the data of credential will be read into database. Once a request of authenticaition is received, the query will be checked if matched with the info in database. Then, sent back the results.

In serverM.cpp, the authentication will be encrypted and exchanged the authentication between serverC and client. The query information and the retrieved result will be exchanged between serverEE/CS and client through serverM.

In serverEE/CS.cpp, the data of courses will be read into database. Once a request of query is received, the info of query will be retrieved from the database and the sent back to main server.

e. The format of all the messages exchanged:
client and main server: 
int length //length of authentication message
char username[200] //authentication message
char a[2] //authentication result
int query_length //length of query message
char query[100] //query message
int reslen1 //length of query result from EE
char res1[1000] //query result from EE
int reslen2 //length of query result from CS
char res2[1000] //query result from CS

main server and serverC:
int length //length of authentication message
char username[200] //authentication message
char a[2] //authentication result

main server and serverCS/serverEE:
int query_length //length of query message
char query[100] //query message
int reslen1 //length of query result from EE
char res1[1000] //query result from EE
int reslen2 //length of query result from CS
char res2[1000] //query

f.Idiosyncrasy of the project:
Interruption of the server process may cause the socket continue occupying the port. Thus the server cannot be booted up since the bind address is still in use..

g.Reused Code:
https://beej.us/guide/bgnet/html/split/index.html

I have reused the code from "Beej's guide to socket programming", and functions include getting the address information of a host, binding the host to a port, connecting the hosts, accepting the connection, sending and receiving the messages.
In detail, these functions are getaddrin(), socket(), bind(), connect(), listen(), accept(), send()/sendto(), recv()/recvfrom().
