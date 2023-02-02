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

#define MYPORT "23678"	// the port users will be connecting to

#define MAXBUFLEN 100

//data struct
struct course{
    char credit[2];
    char prof[50];
    char days[10];
    char name[50];
};

// get sockaddr, IPv4 or IPv6:
//https://beej.us/guide/bgnet/html/#a-simple-stream-server
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

    //Read the ee file
    map<string, struct course> db;
    struct course courses[10];
    ifstream infile("ee.txt");
    int step=0;
    while (infile)
    {
        string s;
        if (!getline(infile, s)) break;

        istringstream ss(s);
        vector <string> record;
        while (ss)
        {
            string s;
            if (!getline(ss, s, ',')) break;
            record.push_back(s);
        }
        strcpy(courses[step].credit,record[1].c_str());
        strcpy(courses[step].prof,record[2].c_str());
        strcpy(courses[step].days,record[3].c_str());
        strcpy(courses[step].name,record[4].c_str());
        db.insert(make_pair(record[0],courses[step]));
        step+=1;
    }
    if (!infile.eof())
    {
        cerr <<"Fail!\n";
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

    freeaddrinfo(servinfo);

    cout<<"The ServerEE is up and running using UDP on port "<<MYPORT<<"."<<endl;

    while(1) {
        int qlen;
        addr_len = sizeof their_addr;

        //Recv the length of courses to query(i.e multiple or single)
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        if ((numbytes = recvfrom(sockfd, &qlen, 4, 0,
                                 (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        int query_length;
        //Recv the query from main server
        //Recv the length of query
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        if ((numbytes = recvfrom(sockfd, &query_length, 4, 0,
                                 (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        //Recv the query message
        //https://beej.us/guide/bgnet/html/#a-simple-stream-server
        char query[100];
        if ((numbytes = recvfrom(sockfd, query, query_length + 1, 0,
                                 (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        //Retrieve the courses or categories in message
        query[numbytes] = '\0';
        string q = query;
        vector<string> record;
        istringstream buf(q);
        while (buf) {
            string r;
            if (!getline(buf, r, ' ')) break;
            record.push_back(r);
        }

        if(qlen==1) { //single course
            string course = record[0];
            string category = record[1];
            cout<<"The ServerEE received a request from the Main Server about the "<<category<<" of "<<course<<"."<<endl;

            //Retrieve the info of query
            char info[100];
            int length;
            if (db.find(course) == db.end()) { //course does not exist
                cout << "Didn't find the course " << course << endl;

                //sent back the info to the main server
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                length = 0;
                if ((numbytes = sendto(sockfd, &length, 4, 0,
                                       (struct sockaddr *) &their_addr, addr_len)) == -1) {
                    perror("talker: sendto");
                    exit(1);
                }
                if ((numbytes = sendto(sockfd, info, strlen(info)+1, 0,
                                       (struct sockaddr *) &their_addr, addr_len)) == -1) {
                    perror("talker: sendto");
                    exit(1);
                }

            } else {
                if (category == "Credit")
                    strcpy(info, db[course].credit);
                else if (category == "Professor")
                    strcpy(info, db[course].prof);
                else if (category == "Days")
                    strcpy(info, db[course].days);
                else if (category == "CourseName")
                    strcpy(info, db[course].name);

                cout<<"The course information has been found: The "<<category<<" of "<<course<<" is "<<info<<"."<<endl;

                //sent back the info to the main server
                //https://beej.us/guide/bgnet/html/#a-simple-stream-server
                length = strlen(info);
                if ((numbytes = sendto(sockfd, &length, 4, 0,
                                       (struct sockaddr *) &their_addr, addr_len)) == -1) {
                    perror("talker: sendto");
                    exit(1);
                }
                if ((numbytes = sendto(sockfd, info, length + 1, 0,
                                       (struct sockaddr *) &their_addr, addr_len)) == -1) {
                    perror("talker: sendto");
                    exit(1);
                }

            }

            cout << "The ServerEE finished sending the response to the Main Server." << endl;
        }
        else{ //multiple courses query
            //Retrieve the info of courses
            char res[1000];
            strcpy(res,"");
            for(int i=0;i<qlen;i++){
                string x=record[i];
                if (db.find(x) != db.end()){
                    strcat(res,"\n");
                    strcat(res, x.c_str());
                    strcat(res,": ");
                    strcat(res,db[x].credit);
                    strcat(res,", ");
                    strcat(res,db[x].prof);
                    strcat(res,", ");
                    strcat(res,db[x].days);
                    strcat(res,", ");
                    strcat(res,db[x].name);

                }
            }

            //sent back the info to the main server
            //https://beej.us/guide/bgnet/html/#a-simple-stream-server
            int length=strlen(res);
            if ((numbytes = sendto(sockfd, &length, 4, 0,
                                   (struct sockaddr *) &their_addr, addr_len)) == -1) {
                perror("talker: sendto");
                exit(1);
            }

            if ((numbytes = sendto(sockfd, res, length+1, 0,
                                   (struct sockaddr *) &their_addr, addr_len)) == -1) {
                perror("talker: sendto");
                exit(1);
            }

        }

    }
    //freeaddrinfo(servinfo);
    close(sockfd);

    return 0;
}