//
//  main.cpp
//  TCPServer
//
//  Created by Muhammad Hussain on 3/28/20.
//  Copyright © 2020 Muhammad Hussain. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define SPORT 5511

using namespace std;

int cmpCommand(char* arr, string comp) {
    // check each char if equal
    int eq = 1;
    for (int i = 0; i < comp.length(); i++) {
        if (arr[i] == comp[i]) {
            eq = 0;
        }
        else {
            eq = 1;
            return eq;
        }
    }
    return eq;
}

string easySystem(const char* cmd) {
    // make an array and string to hold data
    array<char, 500> lsbuff;
    string out;
    
    // go through the system command, copy data to string
    unique_ptr<FILE, decltype(&pclose)> content(popen(cmd, "r"), pclose);
    while (fgets(lsbuff.data(), lsbuff.size(), content.get()) != nullptr) {
        out = out + lsbuff.data();
    }
    return out;
}

string getFileName(char* arr, int skips) {
    string finalString = "";
    
    int i = 0;
    
    // skip a word or two
    for (int k = 0; k < skips; k++) {
        i++;
        char curChar = arr[i];
        while (curChar != ' ') {
            i++;
            curChar = arr[i];
        }
    }
    
    // make the filename by adding chars
    i++;
    char endChar = arr[i];
    while (endChar != ' ' && endChar != NULL) {
        finalString += arr[i];
        i++;
        endChar = arr[i];
    }
    
    
    return finalString;
}

int main(int argc, const char * argv[]) {
        
    // make socket
    int sockfd0 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd0 == -1) {
        perror("socket");
        return 1;
    }
    
    // bind socket
    struct sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(SPORT);
    sAddr.sin_addr.s_addr = INADDR_ANY;
    memset(sAddr.sin_zero, '\0', sizeof sAddr.sin_zero);
    
    int binding;
    binding = ::bind(sockfd0, (struct sockaddr*)&sAddr, sizeof(sAddr));
    if (binding == -1) {
        perror("binding");
        return 2;
    }
    
    // listen
    int listening = listen(sockfd0, 5);
    if (listening == -1) {
        perror("listen");
        return 3;
    }
    
    // for client
    int cSock;
    struct sockaddr_in cAddr;
    socklen_t aSize;
    
    string line;
    char bufferin[5000];
    char bufferout[5000];
    
    // for child proc
    pid_t childp;
    
    while (1) {
        // accept a connection
        cout << "server waiting for  connections"  << endl;
        cSock = accept(sockfd0, (struct sockaddr*)&cAddr, &aSize);
        if (cSock == -1) {
            perror("accept");
            return 4;
        }
        cout << "connection from " << inet_ntoa(cAddr.sin_addr) << endl;
        
        // child process
        childp = fork();
        if (childp == 0) {
            close(sockfd0);
            
            // loop child
            while (1) {
                
                // clear buffers
                bzero(bufferin, sizeof(bufferin));
                bzero(bufferout, sizeof(bufferout));
                
                // wait for a request
                cout << "waiting for request from " << inet_ntoa(cAddr.sin_addr) << endl;
                
                // recieve data
                int rec = recv(cSock, bufferin, 5000, 0);
                if (rec == -1) {
                    perror("rec");
                    return 5;
                }
                
                if(strcmp(bufferin, "catalog") == 0){
                    cout << "got catalog request" << endl;
                    string str = easySystem("ls");
                    strcpy(bufferout, str.c_str());
                    cout << "sending:" << bufferout << endl;
                    send(cSock, bufferout, strlen(bufferout), 0);
                }
                else if(strcmp(bufferin, "spwd") == 0){
                    cout << "got pwd request" << endl;
                    string str = easySystem("pwd");
                    strcpy(bufferout, str.c_str());
                    cout << "sending:" << bufferout << endl;
                    send(cSock, bufferout, strlen(bufferout), 0);
                }
                else if(strcmp(bufferin, "bye") == 0){
                    cout << "got bye request" << endl;
                    cout << "disconnection from " << inet_ntoa(cAddr.sin_addr) << endl;
                    cout << "File copy server is down!" << endl;
                    // close(cSock);
                    // cout << "c sock closed!" << endl;
                    break;
                }
                else if(cmpCommand(bufferin, "download") == 0){
                    // download source filename dest filename
                    //
                    
                    cout << "got download request" << endl;
                    
                    // get name of source filename
                    string sname = getFileName(bufferin, 1);
                    
                    cout << "for " << sname << endl;
                    
                    ifstream infile (sname);
                    
                    // send source file data if valid
                    if (infile.is_open()) {
                        string line;
                        // int i = 0;
                        string allData( (std::istreambuf_iterator<char>(infile)),(std::istreambuf_iterator<char>()));
                        strcpy(bufferout, allData.c_str());
                        cout << "sending data" << endl;
                        send(cSock, bufferout, strlen(bufferout), 0);
                    } else {
                        string str = "didn't get valid filename";
                        cout << str << endl;
                        str = "----";
                        strcpy(bufferout, str.c_str());
                        send(cSock, bufferout, strlen(bufferout), 0);
                    }
                    
                }
                else if(cmpCommand(bufferin, "upload") == 0){
                    // get request, save destination file
                    string dName = getFileName(bufferin, 2);
                    bzero(bufferin, sizeof(bufferin));
                    bzero(bufferout, sizeof(bufferout));
                    
                    // get data
                    int rec = recv(cSock, bufferin, 5000, 0);
                    //    clear buffers
                    ifstream outfile (dName);
                    string str;
                    if (outfile.is_open()) { // overwrite file
                        outfile.close();
                        
                        cout << "overwriting " << dName << endl;
                        str = "file overwritten!";
                        
                        ofstream output (dName, std::ofstream::trunc);
                        output << bufferin << endl;
                        outfile.close();
                    }
                    else { // make a new file
                        cout << "making " << dName << endl;
                        
                        // copy data over to new file
                        ofstream output (dName, std::ofstream::trunc);
                        output << bufferin << endl;
                        output.close();
                        
                        cout << "finished copy" << endl;
                        str = "made new file, finished copy!";
                    }
                    
                    strcpy(bufferout, str.c_str());
                    send(cSock, bufferout, strlen(bufferout), 0);
                }
                else {
                    cout << "didn't get valid input" << endl;
                    string str = "didn't get valid input";
                    strcpy(bufferout, str.c_str());
                    send(cSock, bufferout, strlen(bufferout), 0);
                }
                
                
            }
        }
    }
    
    return 0;
}
