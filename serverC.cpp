//
// Created by student on 11/22/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

#define MYPORT "21678"	// the port users will be connecting to

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    map<string, string> db;

    //read the cred file
    string line;
    ifstream infile("cred.txt");
    while (getline(infile, line))
    {
        istringstream buf(line);
        string record0, record1;
        getline(buf, record0, ',');
        getline(buf, record1, '\r');
        db.insert(make_pair(record0, record1));
    }

    //code copied from beej.us
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    freeaddrinfo(servinfo);
    cout<<"The ServerC is up and running using UDP on port "<<MYPORT<<"."<<endl;

    while(1){
        char auth_info[120];
        int length;

        //Recv the auth from main server
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        addr_len = sizeof their_addr;
        //Recv the auth info length
        if ((numbytes = recvfrom(sockfd, &length, 4 , 0,
                                 (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        //Recv the auth message
        if ((numbytes = recvfrom(sockfd, auth_info, length+1 , 0,
                                 (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        auth_info[numbytes]='\0';
        cout<<"The ServerC received an authentication request from the Main Server."<<endl;

        //Retrieve the username and password in message
        vector<string> record;
        string auth_string=auth_info;
        istringstream buf(auth_string);
        while(buf){
            string r;
            if(!getline(buf,r,'\n')) break;
            record.push_back(r);
        }

        string name=record[0];
        string code=record[1];

        //Check the username and password
        char auth[2];
        if(db.find(name)==db.end()){ //if not found username
            strcpy(auth,"1");
        }
        else{
            if(code.compare(db[name])==0) //match the password
                strcpy(auth,"0");
            else //not match the password
                strcpy(auth,"2");
        }

        //sent the auth result back to main server
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        if ((numbytes = sendto(sockfd, auth, 2, 0,
                              (struct sockaddr *)&their_addr, addr_len)) == -1) {
            perror("talker: sendto");
            exit(1);
        }

        cout<<"The ServerC finished sending the response to the Main Server."<<endl;
        //freeaddrinfo(servinfo);

    }
    close(sockfd);
    return 0;
}