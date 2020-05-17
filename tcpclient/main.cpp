//
//  main.cpp
//  TCPClient
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define SPORT 5511

using namespace std;

int cmpCommand(char* arr, string comp) {
    // check each char for equality
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
    // build this string
    string finalString = "";
    
    int i = 0;
    
    // get rid of first one or two words
    for (int k = 0; k < skips; k++) {
        i++;
        char curChar = arr[i];
        while (curChar != ' ') {
            i++;
            curChar = arr[i];
        }
    }
    
    i++;
    char endChar = arr[i];
    while (endChar != ' ' && endChar != NULL) {
        // add chars to string
        finalString += arr[i];
        i++;
        endChar = arr[i];
    }
    
    
    return finalString;
}

int main(int argc, const char * argv[]) {
    
    // check for args
    if (argc != 2) {
        cout << "invalid number of arguments" << endl;
        return 1;
    }
    
    // make socket
    int sockfd1;
    sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd1 == -1) {
        return 2;
    }
    
    // get the server
    struct hostent *hostptr;
    hostptr = gethostbyname(argv[1]);
    if (hostptr == nullptr) {
        return 3;
    }
    
    struct sockaddr_in sAddr;
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(SPORT);
	sAddr.sin_addr = *((struct in_addr *)hostptr->h_addr);
    memset(&sAddr.sin_zero, '\0', sizeof(sAddr).sin_zero);
    
    // connect to server
    int con = connect(sockfd1, (struct sockaddr*)&sAddr, sizeof(sAddr));
    if (con == -1) {
        return 4;
    }
    
    // for communication
    char bufferin[5000];
    char bufferout[5000];
    
    // loop client services
    while(1){
        // clear buffers
        bzero(bufferin, sizeof(bufferin));
        bzero(bufferout, sizeof(bufferout));
        
        // ask for a command
        cout << "Commands:" << endl;
        cout << "ls" << endl;
        cout << "pwd" << endl;
        cout << "catalog" << endl;
        cout << "spwd" << endl;
        cout << "bye" << endl;
        cout << "upload source destination" << endl;
        cout << "download source destination" << endl;
        cout << "Enter command:" << endl;
        string inputStr;
        getline(cin, inputStr);
        strcpy(bufferout, inputStr.c_str());
        
        
        if(strcmp(bufferout, "ls") == 0){
            cout << easySystem("ls") << endl;
        }
        else if(strcmp(bufferout, "pwd") == 0){
            cout << easySystem("pwd") << endl;
        }
        else if(strcmp(bufferout, "catalog") == 0){
            send(sockfd1, bufferout, strlen(bufferout), 0);
            
            // recieve catalog data
            int rec = recv(sockfd1, bufferin, 5000, 0);
            if (rec == -1) {
                return 5;
            }
            // print recieved data
            cout << bufferin << endl;
        }
        else if(strcmp(bufferout, "spwd") == 0){
            send(sockfd1, bufferout, strlen(bufferout), 0);
            
            // get pwd data
            int rec = recv(sockfd1, bufferin, 5000, 0);
            if (rec == -1) {
                return 5;
            }
            cout << bufferin << endl;
        }
        else if(strcmp(bufferout, "bye") == 0){
            cout << "sending bye request, disconnecting" << endl;
            cout << "Internet copy client is down!" << endl;
            // disconnect
            send(sockfd1, bufferout, strlen(bufferout), 0);
            close(sockfd1);
            return 0;
        }
        else if(cmpCommand(bufferout, "download") == 0) {
            cout << "sent download request" << endl;
            
            // send download file names
            send(sockfd1, bufferout, strlen(bufferout), 0);
            int rec = recv(sockfd1, bufferin, 5000, 0);
            
            cout << "got this data: " << bufferin << endl;
            
            // check if this is a valid file
            bool validfile = true;
            if ( bufferin[0] == '-') {
                cout << "didn't send valid download filename" << endl;
                validfile = false;
            }
            if (!validfile) {
                // cancel download
                cout << bufferin << endl;
                continue;
            }
            
            // where to put this data
            string destName = getFileName(bufferout, 2);
            
            ifstream outfile (destName);
            
            // check if this is a new file or an overwrite
            if (outfile.is_open()) {
                
                outfile.close();
                
                cout << "overwriting " << destName << endl;
                
                ofstream output (destName, std::ofstream::trunc);
                output << bufferin << endl;
                
                cout << "finished copy" << endl;
            } else {
                // make a file
                cout << "making " << destName << endl;
                
                // copy data over to new file
                ofstream output (destName, std::ofstream::trunc);
                output << bufferin << endl;
                
                cout << "finished copy" << endl;
            }
            
            // cout << bufferin << endl;
        }
        else if(cmpCommand(bufferout, "upload") == 0) {
            // send file name data
            string sourceName = getFileName(bufferout, 1);
            
            // check if we have this file
            ifstream infile (sourceName);
            if (infile.is_open()) {
                // send request
                send(sockfd1, bufferout, strlen(bufferout), 0);
                // send data
                string allData( (std::istreambuf_iterator<char>(infile)),(std::istreambuf_iterator<char>()));
                //     clear buffers
                bzero(bufferin, sizeof(bufferin));
                bzero(bufferout, sizeof(bufferout));
                strcpy(bufferout, allData.c_str());
                cout << "sending data, sent: " << endl;
                cout << bufferout << endl;
                send(sockfd1, bufferout, strlen(bufferout), 0);
                // get response from server
                int rec = recv(sockfd1, bufferin, 5000, 0);
                cout << bufferin << endl;
            } else {
                cout << "error, no such source file" << endl;
            }
        }
        else {
            // not a valid input
            send(sockfd1, bufferout, strlen(bufferout), 0);
            int rec = recv(sockfd1, bufferin, 5000, 0);
            cout << bufferin << endl;
        }
        
        // cout << bufferin << endl;
	}
    
    
    
    
    
    
    
    
    
    
    cout << "bye bye bye" << endl;
    return 0;
}
