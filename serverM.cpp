/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include<iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

#define CPORT "21678"  // the port users will be connecting to
#define CSPORT "22678"  // the port users will be connecting to
#define EEPORT "23678"  // the port users will be connecting to
#define PORT "25678"
#define UDP "24678"
#define BACKLOG 10	 // how many pending connections queue will hold

//https://beej.us/guide/bgnet/html/#a-simple-stream-server
void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
//https://beej.us/guide/bgnet/html/#a-simple-stream-server
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//Encryption method
string encrypt(string text){
    for(int i=0;i<text.size();i++){
        if(isalpha(text[i])){
            if(islower(text[i])) {
                text[i] = 'a' + (text[i] - 'a' + 4) % 26;
            }
            else if(isupper(text[i])){
                text[i]='A' + (text[i] - 'A' + 4) % 26;
            }
        }
        else if(isdigit(text[i])){
            text[i]='0'+ (text[i]-'0'+4)%10;
        }
    }
    return text;
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int numbytes;

    //code copied from beej.us
    //TCP
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    //UDP
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    int sockfd2;
    struct addrinfo hints2, *servinfo2, *p2;
    int rv2;
    struct sockaddr_storage their_addr2;
    socklen_t addr_len2;
    char s2[INET6_ADDRSTRLEN];

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints2.ai_socktype = SOCK_DGRAM;
    hints2.ai_flags = AI_PASSIVE; // use my IP

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv2 = getaddrinfo(NULL, UDP, &hints2, &servinfo2)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
        return 1;
    }

    // loop through all the results and make a socket
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
        if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
                              p2->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        if (bind(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
            close(sockfd2);
            perror("listener: bind");
            continue;
        }

        break;
    }
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if (p2 == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
    freeaddrinfo(servinfo2);

    //UDP talk to c
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    struct addrinfo hints3, *servinfo3, *p3;
    int rv3;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints3, 0, sizeof hints3);
    hints3.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints3.ai_socktype = SOCK_DGRAM;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv3 = getaddrinfo("127.0.0.1", CPORT, &hints3, &servinfo3)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv3));
        return 1;
    }

    p3= servinfo3;

    //UDP TALK TO EE
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    struct addrinfo hints4, *servinfo4, *p4;
    int rv4;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints4, 0, sizeof hints4);
    hints4.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints4.ai_socktype = SOCK_DGRAM;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv4 = getaddrinfo("127.0.0.1", EEPORT, &hints4, &servinfo4)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv4));
        return 1;
    }

    p4= servinfo4;

    //UDP TALK TO CS
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    struct addrinfo hints5, *servinfo5, *p5;
    int rv5;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints5, 0, sizeof hints5);
    hints5.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints5.ai_socktype = SOCK_DGRAM;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv5 = getaddrinfo("127.0.0.1", CSPORT, &hints5, &servinfo5)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv5));
        return 1;
    }

    p5= servinfo5;

    cout<<"The main server is up and running."<<endl;

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);

        int authentication=-1;
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            //Authentication process
            while(authentication!=0) {
                //Recv auth from client
                int length;
                char auth_info[120];
                if ((numbytes = recv(new_fd, &length, 4, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                if ((numbytes = recv(new_fd, auth_info, length+1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                //encrypt the message
                auth_info[numbytes]='\0';;
                vector<string> record;
                vector<string> origin;
                string auth_string=auth_info;
                istringstream buf(auth_string);
                while(buf){
                    string r;
                    if(!getline(buf,r,'\n')) break;
                    origin.push_back(r);
                    record.push_back(encrypt(r));
                }

                cout<<"The main server received the authentication for "<<origin[0]<<" using TCP over port "<<PORT<<"."<<endl;

                //concatanate the message
                char username[120];
                strcpy(username, "");
                strcat(username, record[0].c_str());
                strcat(username, "\n");
                strcat(username, record[1].c_str());

                //sent the message to serverC
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                int length2=strlen(username);
                if ((numbytes = sendto(sockfd2, &length2, 4 , 0,
                                       p3->ai_addr, p3->ai_addrlen)) == -1) {
                    perror("talker: sendto");
                    exit(1);
                }
                if ((numbytes = sendto(sockfd2, username , length2+1,0,
                                       p3->ai_addr, p3->ai_addrlen)) == -1) {
                    perror("talker: sendto");
                    exit(1);
                }
                cout<<"The main server sent the authentication request to serverC."<<endl;

                //Recv the auth result from ServerC
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                char auth[2];
                addr_len2 = sizeof their_addr2;
                if ((numbytes = recvfrom(sockfd2, auth, 2, 0,
                                         (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                auth[numbytes]='\0';
                int au;
                au=int(auth[0]-'0');
                cout<<"The main server received the result of the authentication request from the ServerC using UDP over port "<<UDP<<"."<<endl;

                //sent the auth result to the client
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                if (send(new_fd, auth ,2, 0) == -1)
                    perror("send");
                cout<<"The main server sent the authentication result to the client."<<endl;

                //Authenticated client
                while(au==0){

                    //Recv the multi-courses request or single course request message
                    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                    int qlen;
                    if ((numbytes = recv(new_fd, &qlen, 4, 0)) == -1) {
                        perror("recv");
                        exit(1);
                    }

                    if(qlen==1){// single course request message
                        //Recv the dept message
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        int dept;
                        if ((numbytes = recv(new_fd, &dept, 4, 0)) == -1) {
                            perror("recv");
                            exit(1);
                        }

                        //Recv the query length message
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        int query_length;
                        if ((numbytes = recv(new_fd, &query_length, 4, 0)) == -1) {
                            perror("recv");
                            exit(1);
                        }

                        //Recv the query message
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        char query[100];
                        if ((numbytes = recv(new_fd, query, query_length+1, 0)) == -1) {
                            perror("recv");
                            exit(1);
                        }
                        query[numbytes]='\0';
                        vector<string> cc;
                        string auth_string=auth_info;
                        istringstream buf(query);
                        while(buf){
                            string r;
                            if(!getline(buf,r,' ')) break;
                            cc.push_back(r);
                        }
                        cout<<"The main server received from "<<origin[0]<<" to query course "<<cc[0]<<" about "<<cc[1]<<" using TCP over port "<<PORT<<endl;

                        if(dept==0) { // send query to EE dept.
                            cout<<"The main server sent a request to serverEE."<<endl;
                            if ((numbytes = sendto(sockfd2, &qlen, 4, 0,
                                                   p4->ai_addr, p4->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                            }
                            if ((numbytes = sendto(sockfd2, &query_length, 4, 0,
                                                   p4->ai_addr, p4->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                            }
                            if ((numbytes = sendto(sockfd2, query, query_length+1, 0,
                                                   p4->ai_addr, p4->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                            }
                        }
                        else{ //send query to CS dept.
                            cout<<"The main server sent a request to serverCS."<<endl;
                            if ((numbytes = sendto(sockfd2, &qlen, 4, 0,
                                                   p5->ai_addr, p5->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                            }
                            if ((numbytes = sendto(sockfd2, &query_length, 4, 0,
                                                   p5->ai_addr, p5->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                            }
                            if ((numbytes = sendto(sockfd2, query, query_length+1, 0,
                                                   p5->ai_addr, p5->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                            }

                        }

                        int reslen;
                        if ((numbytes = recvfrom(sockfd2, &reslen, 4, 0,
                                                 (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                            perror("recvfrom");
                            exit(1);
                        }
                        char res[100];
                        if ((numbytes = recvfrom(sockfd2, res, reslen+1, 0,
                                                 (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                            perror("recvfrom");
                            exit(1);
                        }

                        if(dept==0)
                            cout << "The main server received the response from serverEE using UDP over port "<<UDP<<"."<< endl;
                        else
                            cout << "The main server received the response from serverCS using UDP over port "<<UDP<<"."<< endl;

                        //sent info to client
                        if (send(new_fd, &reslen, 4, 0) == -1)
                            perror("send");
                        //sent info to client
                        if (send(new_fd, res, reslen+1, 0) == -1)
                            perror("send");
                        cout<<"The main server send the query information to the client."<<endl;

                    }
                    else{
                        //Recv query from client.
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        int query_length;
                        if ((numbytes = recv(new_fd, &query_length, 4, 0)) == -1) {
                            perror("recv");
                            exit(1);
                        }

                        char query[100];
                        if ((numbytes = recv(new_fd, query, query_length+1, 0)) == -1) {
                            perror("recv");
                            exit(1);
                        }
                        query[numbytes]='\0';

                        //Send query to EE dept.
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        if ((numbytes = sendto(sockfd2, &qlen, 4, 0,
                                               p4->ai_addr, p4->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        if ((numbytes = sendto(sockfd2, &query_length, 4, 0,
                                               p4->ai_addr, p4->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        if ((numbytes = sendto(sockfd2, query, query_length+1, 0,
                                               p4->ai_addr, p4->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        //Recv info from EE dept.
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        int reslen1;
                        if ((numbytes = recvfrom(sockfd2, &reslen1, 4, 0,
                                                 (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                            perror("recvfrom");
                            exit(1);
                        }
                        char res1[1000];
                        if ((numbytes = recvfrom(sockfd2, res1, reslen1+1, 0,
                                                 (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                            perror("recvfrom");
                            exit(1);
                        }
                        //Send info to client
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        if (send(new_fd, &reslen1, 4, 0) == -1)
                            perror("send");
                        if (send(new_fd, res1, reslen1+1, 0) == -1)
                            perror("send");

                        //Send query to CS dept.
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        if ((numbytes = sendto(sockfd2, &qlen, 4, 0,
                                               p5->ai_addr, p5->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        if ((numbytes = sendto(sockfd2, &query_length, 4, 0,
                                               p5->ai_addr, p5->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        if ((numbytes = sendto(sockfd2, query, query_length+1, 0,
                                               p5->ai_addr, p5->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        //Recv info from CS dept.
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        int reslen2;
                        if ((numbytes = recvfrom(sockfd2, &reslen2, 4, 0,
                                                 (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                            perror("recvfrom");
                            exit(1);
                        }
                        char res2[1000];
                        if ((numbytes = recvfrom(sockfd2, res2, reslen2+1, 0,
                                                 (struct sockaddr *) &their_addr2, &addr_len2)) == -1) {
                            perror("recvfrom");
                            exit(1);
                        }
                        //Send query to CS dept.
                        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                        if (send(new_fd, &reslen2, 4, 0) == -1)
                            perror("send");
                        if (send(new_fd, res2, reslen2+1, 0) == -1)
                            perror("send");
                    }


                }
            }

            close(new_fd);
            exit(0);

        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
