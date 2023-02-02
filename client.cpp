//
// Created by leo on 11/19/2022.
//

/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include<iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

#define PORT "25678" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
//https://beej.us/guide/bgnet/html/#a-simple-stream-server
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

const string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string rtrim(const string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    //code copied from beej.us
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if ((rv = getaddrinfo("127.0.0.1",PORT , &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }
    //https://beej.us/guide/bgnet/html/#a-simple-stream-server
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //PIAZZA:get the local dynamic port number
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    int port_num;
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    else
        port_num=ntohs(sin.sin_port);

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);

    cout << "The client is up and running." << endl;
    freeaddrinfo(servinfo); // all done with this structure

    int attempt = 3;
    int auth;
    char username[120];
    char password[50];
    //Authentication process
    while(attempt>=1) {
        cout << "Please enter the username: ";
        cin.getline(username, 50);
        cout << "Please enter the password: ";
        cin.getline(password, 50);

        //concatanate the message
        string user=username;
        strcat(username, "\n");
        strcat(username, password);
        cout<< user << " sent an authentication request to the main server."<<endl;

        //send auth to main server
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        int length=strlen(username);
        if (send(sockfd, &length ,4, 0) == -1)
            perror("send");

        if (send(sockfd, username, strlen(username)+1, 0) == -1)
            perror("send");

        //Recv auth result from main server
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        char a[2];
        if ((numbytes = recv(sockfd, a, 2, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        a[numbytes]='\0';

        auth = int(a[0] - '0');
        switch(auth){
            case 0:
                cout<< user<< " received the result of authentication using TCP over port "<<port_num<<". Authentication is successful."<<endl;
                break;
            case 1:
                cout<< user << " received the result of authentication using TCP over port "<<port_num<<". Authentication failed: Username does not exist."<<endl;
                cout<<"Attempts remaining: "<< attempt-1 << endl;
                break;
            case 2:
                cout<< user << " received the result of authentication using TCP over port "<<port_num<<".  Authentication failed: Password does not match."<<endl;
                cout<<"Attempts remaining: "<< attempt-1 << endl;
                break;
        }

        while(auth==0){
            char query[100];
            char category[20];
            vector <string> record;
            cout<<"Please enter the course code to query: ";
            cin.getline(query,100);
            string q=query;
            q=trim(q);

            //get the number of courses
            istringstream buf(q);
            while(buf){
                string r;
                if(!getline(buf,r,' ')) break;
                record.push_back(r);
            }

            int qlen=record.size();

            if (send(sockfd, &qlen, 4, 0) == -1)
                perror("send");

            if(qlen==1){ //single course query
                cout<<"Please enter the category(Credit/Professor/Days/CourseName): ";
                cin.getline(category, 20);
                string cate=category;
                cate=trim(cate);
                char qu[100];
                strcpy(qu,q.c_str());
                strcat(qu, " ");
                strcat(qu, cate.c_str());
                int dept;
                if(q.substr(0,2).compare("EE")==0) {
                    dept = 0;
                }
                else
                    dept=1;
                cout<< user << " sent a request to the main server."<<endl;
                if (send(sockfd, &dept ,4, 0) == -1)
                    perror("send");

                int query_length=strlen(qu);
                if (send(sockfd, &query_length ,4, 0) == -1)
                    perror("send");

                if (send(sockfd, qu, query_length+1, 0) == -1)
                    perror("send");


                int reslen;
                //Recv auth from main server
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                if ((numbytes = recv(sockfd, &reslen, 4, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                char res[100];
                //Recv auth from main server
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                if ((numbytes = recv(sockfd, res, reslen+1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                res[numbytes]='\0';
                cout<<"The client received the response from the Main server using the TCP over port "<<port_num<<"."<<endl;
                if (reslen==0)
                    cout<<"Didn't find the course "<<q<<endl;
                else
                    cout<<"The "<<category<<" of "<<q<<" is "<< res<<"."<< endl;
                cout<<"\n----Start a new request----"<<endl;

            }
            else{
                cout<< user<<" sent a request with multiple CourseCode to the main server."<<endl;
                cout<<"CourseCode: Credits, Professor, Days, CourseName";
                char qu[100];
                strcpy(qu,q.c_str());
                int query_length= strlen(qu);
                if (send(sockfd, &query_length, 4, 0) == -1)
                    perror("send");
                if (send(sockfd, qu, query_length+1, 0) == -1)
                    perror("send");

                int reslen1;
                //Recv auth from main server
                if ((numbytes = recv(sockfd, &reslen1, 4, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                char res1[1000];
                //Recv auth from main server
                if ((numbytes = recv(sockfd, res1, reslen1+1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                res1[numbytes]='\0';
                cout<<res1;

                int reslen2;
                //Recv auth from main server
                if ((numbytes = recv(sockfd, &reslen2, 4, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                char res2[1000];
                //Recv auth from main server
                if ((numbytes = recv(sockfd, res2, reslen2+1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                res2[numbytes]='\0';
                cout<<res2<<endl;
                cout<<"\n----Start a new request----"<<endl;
            }
        }
        attempt-=1;
    }
    if(auth!=0 and attempt==0){
        cout<< "Authentication failed for 3 attempts. Client will shut down."<< endl;
        close(sockfd);
        exit(1);
    }

    close(sockfd);

    return 0;
}
