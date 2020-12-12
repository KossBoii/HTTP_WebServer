#include "server.h"
#include <iostream>
#include <fstream>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "diamondhttp.h"
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"

using namespace std;

Server::Server(){   // CTORs
    memset(&(this->inVal), 0, sizeof(this->inVal));
    (this->inVal).ai_family = AF_UNSPEC;
    (this->inVal).ai_socktype = SOCK_STREAM;
    (this->inVal).ai_flags = AI_PASSIVE;
    yes = 1;
}

void helper_func(int signal){
    // save question_id_to_fname to file
    cout<<"\nsave question_id_to_fname to file "<<(root_dir + "/config.txt")<<"\n";
    ofstream wFile;

    wFile.open((root_dir + "/config.txt").c_str(), ios::trunc);

    if(wFile.is_open()){
        map<int, string>::iterator it;
        for(it=question_id_to_fname.begin(); it != question_id_to_fname.end(); it++){
            wFile<<it->first<<"->"<<it->second<<endl;
        }
        wFile.close();
    } else{
        cout<<"file does not open\n";
    }
    exit(signal);
}

void Server::accept_connection(){
    if(getaddrinfo(NULL, port.c_str(), &(this->inVal), &(this->sv_info)) != 0){
        perror("get address: ");
    }

    for(this->valid_info = this->sv_info; this->valid_info != NULL; this->valid_info = (this->valid_info)->ai_next){
        // Create socket
        sock_id = socket((this->valid_info)->ai_family, (this->valid_info)->ai_socktype, 0);
        if(sock_id == -1){
            perror("socket: ");
        }

        this->addr_len = sizeof(this->sv_info);

        // Do this to prevent error "address already in use"
        if(setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt: ");
            break;
        }

        // Bind socket
        if(bind(sock_id, (this->valid_info)->ai_addr, (this->valid_info)->ai_addrlen) == -1){
            perror("bind: ");
        }
        break;
    }

    // Successfully binding socket
    struct sockaddr_in* ipv4 = (struct sockaddr_in*) (this->valid_info)->ai_addr;
    void* addr = &(ipv4->sin_addr);
    
    // Convert ip to string format
    inet_ntop((this->valid_info)->ai_family, addr, this->ip, sizeof(this->ip));
    freeaddrinfo(sv_info);

    // Put the server socket in passive mode (waits for the client to make connection to server)
    if(listen(sock_id, MAX_CONNECTIONS) == -1){
        perror("listen: ");
    }

    signal(SIGINT, helper_func);

    while(true) {
        this->address = sizeof(this->client_addr);

        // Get first connection request ==> establish connection between client and server
        this->accept_id = accept(sock_id, (struct sockaddr*)& this->client_addr, (socklen_t*)& this->address);
        if(this->accept_id == -1){
            perror("accept: ");
        }

        inet_ntop((this->client_addr).ss_family, this->get_ip_address((struct sockaddr*)& this->client_addr), this->ip1, sizeof(this->ip1));
        u_int16_t client_port = this->get_port_number((struct sockaddr*)& this->valid_info);
        
        time_t temp_time = time(NULL);
        tm* cur_time = gmtime(&temp_time);
        char cur_time_formatted[50];
        if(strftime(cur_time_formatted, 50, "%x:%X", cur_time) == 0){
            perror("time format error: ");        
        }

        // Setup ClientIdentity for current client
        ClientIdentity client;
        client.accept_id = this->accept_id;
        string temp(this->ip1);
        client.ip = temp;
        client.port_num = client_port;
        client.req_time = cur_time_formatted;

        // Pass the ClientIdentity to Parser
        parser->parseRequest(client);
    }

}

void* Server::get_ip_address(sockaddr* sock){
    if(sock->sa_family == AF_INET){     // IPv4
        return &((struct sockaddr_in*) sock)->sin_addr;
    }
    else{       // IPv6
        return &((struct sockaddr_in6*) sock)->sin6_addr;
    }
}

u_int16_t Server::get_port_number(struct sockaddr* sock){
    if(sock->sa_family == AF_INET){     // IPv4
        return ((struct sockaddr_in*) sock)->sin_port;
    }
    else{       // IPv6
        return ((struct sockaddr_in6*) sock)->sin6_port;
    }
}
