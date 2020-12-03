#include "parser.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include "diamondhttp.h"
#include "data_utils.h"

using namespace std;

bool sortRequest(const ClientInfo& lhs, const ClientInfo& rhs){
    return lhs.req_file_size <= rhs.req_file_size;
}

void Parser::parseRequest(ClientIdentity client){
    int i = 1;
    int recv_bytes = 0;
    char buffer[1024];
    char *p, *p_buffer;
    ClientInfo client_info;

    fcntl(client.accept_id, O_NONBLOCK, 0);

    recv_bytes = recv(client.accept_id, buffer, sizeof(buffer), 0);
    if(recv_bytes == -1){
        perror("receive: ");
    }
    buffer[recv_bytes] = '\0';
    
    string fetch_line(buffer);
    int cur_index = 0;
    int next_index = fetch_line.find_first_of("\r\n", cur_index);

    if(next_index < 0){
        write(client_info.req_accept_id, "Bad Request Error: Retry!", 25);
        close(client_info.req_accept_id);
    } else {
        client_info.req_first_line = fetch_line.substr(cur_index, next_index - cur_index);
        string parseTokens[3];

        p_buffer = new char[client_info.req_first_line.size() + 1];
        copy(client_info.req_first_line.begin(), client_info.req_first_line.end(), p_buffer);
        p_buffer[client_info.req_first_line.size()] = '\0';

        p = strtok(p_buffer, " /");
        parseTokens[0] = p;
        while(i < 3) {
            p = strtok(NULL, " ");
            parseTokens[i] = p;
            i++;
        }

        client_info.req_accept_id = client.accept_id;
        client_info.req_port_num = client.port_num;
        client_info.req_ip = client.ip;
        client_info.req_time = client.req_time;
        client_info.req_type = parseTokens[0];
        client_info.req_filename = parseTokens[1];

        int first_slash_index = client_info.req_filename.find_first_of("/", 0);
        int sec_slash_index = client_info.req_filename.find_first_of("/", first_slash_index + 1);

        if(sec_slash_index >= 0) {
            client_info.root_check = false;
        } else {
            client_info.root_check = true;
        }

        client_info.req_filename = root_dir + client_info.req_filename;
        client_info.req_method = parseTokens[2];

        delete [] p_buffer;
        p_buffer = NULL;
        changeDirectory(client_info);
    }
}

void Parser::changeDirectory(ClientInfo client_info){
    int next = client_info.req_filename.find_first_of("~", 0);
    int size = client_info.req_filename.size();
    if(next > 0 && next < size){
        int index = client_info.req_filename.find_first_of("/", next);
        string request = client_info.req_filename.substr(next + 1, index - (next + 1));
        string rest_str = client_info.req_filename.substr(index, size - index);
        client_info.req_filename.erase(next, size);
        client_info.req_filename = client_info.req_filename + request + "" + rest_str;
    }

    checkRequest(client_info);
}

bool Parser::checkFileExists(string fileName) {
    struct stat checker;
    if(stat(fileName.c_str(), &checker) != -1)
        return true;
    return false;
}

void Parser::requestQueue(ClientInfo client_info) {
    // Main thread will put the client's request to the request queue
    pthread_mutex_lock(&req_queue_lock);
    client_list.push_back(client_info);
    pthread_cond_signal(&req_queue_cond);
    pthread_mutex_unlock(&req_queue_lock);
}

void Parser::popRequest(){
    sleep(sleep_time);
    while(true) {
        ClientInfo client_info;
        transform(schedule_type.begin(), schedule_type.end(), schedule_type.begin(), ::toupper);    // convert schedule_type string to all uppercase

        // Check scheduling type
        if(schedule_type == "SJF"){     // SJF: Shortest Job First
            pthread_mutex_lock(&req_queue_lock);

            while(client_list.empty()){     // no client
                pthread_cond_wait(&req_queue_cond, &req_queue_lock);
            }
            client_list.sort(sortRequest);      // sort the job based on file size
            client_info =  client_list.front();
            client_list.pop_front();
            pthread_mutex_unlock(&req_queue_lock);
        }
        else if(schedule_type == "FCFS") {  // FCFs: First Come First Serve
            pthread_mutex_lock(&req_queue_lock);

            while(client_list.empty()){     // no client
                pthread_cond_wait(&req_queue_cond, &req_queue_lock);
            }

            client_info =  client_list.front();
            client_list.pop_front();
            pthread_mutex_unlock(&req_queue_lock);
        }

        pthread_mutex_lock(&print_lock);
        req_list.push_back(client_info);
        pthread_cond_signal(&print_cond);
        pthread_mutex_unlock(&print_lock);
    }
}

void Parser::processRequest(){
    pthread_detach(pthread_self()); // detach current thread
    while(true){
        pthread_mutex_lock(&print_lock);

        while(req_list.empty()){     // no client request
            pthread_cond_wait(&print_cond, &print_lock);
        }

        ClientInfo client_info;
        client_info = req_list.front();
        req_list.pop_front();

        time_t temp_time = time(NULL);
        tm* cur_time = gmtime(&temp_time);
        char cur_time_formatted[50];
        if(strftime(cur_time_formatted, 50, "%x:%X", cur_time) == 0){
            perror("time format error: ");        
        }
        client_info.req_serve_time = string(cur_time_formatted);
        pthread_mutex_unlock(&print_lock);
        
        // Send Data
        DataUtils d;
        d.sendData(client_info);
    }
}

void* Parser::popRequest_helper(void* client){
    ((Parser*) client)->popRequest();
    return NULL;
}

void* Parser::processRequest_helper(void* client){
    ((Parser*) client)->processRequest();
    return NULL;
}

void Parser::checkRequest(ClientInfo client_info){
    int last_slash_index = client_info.req_filename.find_last_of("/");
    int temp = client_info.req_filename.find_last_of(".");

    if(temp == string::npos){   // can't find '.'
        client_info.req_file_size = 0;
        client_info.status_file = false;
        requestQueue(client_info);
    } else {        // found fileName
        if(checkFileExists(client_info.req_filename)){
            if(client_info.req_type == "GET"){
                ifstream file;
                file.open(client_info.req_filename);
                
                // Set req_file_size
                if(file.is_open()){
                    file.seekg(0, ios::end);
                    client_info.req_file_size = (int) file.tellg();
                } else {
                    client_info.req_file_size = 0;
                }

                client_info.status_file = true;
                file.close();
                requestQueue(client_info);
            } else if(client_info.req_type == "POST"){
                cout<<"POST\n";
            }
        } else{
            client_info.req_file_size = 0;
            client_info.status_file = false;
            requestQueue(client_info);
        }
    }
}

