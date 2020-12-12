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
#include <sstream>
#include <jsoncpp/json/json.h>
#include "diamondhttp.h"
#include "data_utils.h"
#include <boost/algorithm/string/replace.hpp>

using namespace std;

bool sortRequest(const ClientRequest& lhs, const ClientRequest& rhs){
    return lhs.req_file_size <= rhs.req_file_size;
}

void Parser::parseRequest(ClientIdentity client){
    int i = 1;
    int recv_bytes = 0;
    char buffer[30000];
    char *p, *p_buffer;
    ClientRequest client_request;

    fcntl(client.accept_id, O_NONBLOCK, 0);

    // Read the request sent from client when connected
    recv_bytes = recv(client.accept_id, buffer, sizeof(buffer), 0);

    if(recv_bytes == -1){       // error when reading the sent request
        perror("receive: ");
    }
    buffer[recv_bytes] = '\0';
    string fetch_line(buffer);

    // Parsing the sent request
    cout<<"-------------------------------------------------------------------------\n";
    cout<<"Fetch Client Request: \n"<<fetch_line<<endl;
    cout<<"-------------------------------------------------------------------------\n";

    int cur_index = 0;
    int next_index = fetch_line.find_first_of("\r\n", cur_index);

    if(next_index < 0){
        write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
        close(client_request.req_accept_id);
    } else {        // Parsing first line
        client_request.req_first_line = fetch_line.substr(cur_index, next_index - cur_index);
        string parseTokens[4];

        int temp_index = client_request.req_first_line.find_first_of(" /");
        if(temp_index != string::npos){
            // Parse request method
            parseTokens[0] =  client_request.req_first_line.substr(0, temp_index);

            // Parse the API
            int temp_index2 = client_request.req_first_line.find_first_of(" ", temp_index + 1);
            if(temp_index2 != string::npos){
                parseTokens[1] =  client_request.req_first_line.substr(temp_index + 1, temp_index2 - temp_index - 1);
            } else{
                write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
                close(client_request.req_accept_id);
            }
            temp_index = temp_index2;

            // Parse Parse Request Transfer Protocol Type
            temp_index2 = client_request.req_first_line.find_first_of("/", temp_index + 1);
            if(temp_index2 != string::npos){
                parseTokens[2] = client_request.req_first_line.substr(temp_index + 1, temp_index2 - temp_index - 1);
            } else{
                write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
                close(client_request.req_accept_id);
            }
            
            // Parse Request Transfer Protocol version
            parseTokens[3] = client_request.req_first_line.substr(temp_index2 + 1, -1);
        } else {
            write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
            close(client_request.req_accept_id);
        }

        // Register Client Request
        client_request.req_accept_id = client.accept_id;
        client_request.req_port_num = client.port_num;
        client_request.req_ip = client.ip;
        client_request.req_time = client.req_time;

        client_request.req_method = parseTokens[0];
        client_request.req_filename = parseTokens[1];
        client_request.req_type = parseTokens[2];
        client_request.req_version = parseTokens[3];

        // Parsing the RequestBody
        int beginIndex = fetch_line.find_last_of("{");
        int endIndex = fetch_line.find_last_of("}");
        string reqBody;
        if(beginIndex != string::npos && endIndex != string::npos){
            boost::replace_all(fetch_line, "%22", "\"");
            reqBody = fetch_line.substr(beginIndex, endIndex - beginIndex + 1);
            cout<<"reqBody"<<reqBody<<endl;
            Json::Reader reader;
            if(!reader.parse(reqBody, client_request.reqBody))
            {
                cout << "Error parsing the string" << endl;
            }
        }

        // Parse Query String
        int index = client_request.req_filename.find("?");
        if(index != string::npos){      // paramaters are passed by string query
            boost::replace_all(fetch_line, "%22", "\"");
            string query_string = client_request.req_filename.substr(index + 1, -1);
            client_request.req_filename = client_request.req_filename.substr(0, index);
            
            if(query_string.length() == 0){
                write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
                close(client_request.req_accept_id);
            } else{
                if(query_string.find("&") == string::npos){     // only has 1 argument
                    int temp_index = query_string.find("=");
                    if(temp_index == string::npos){
                        write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
                        close(client_request.req_accept_id);
                    } else{
                        client_request.reqBody[query_string.substr(0, temp_index)] = query_string.substr(temp_index + 1, -1);
                    }
                    cout<<"requestBody: "<<client_request.reqBody<<endl;
                } else {                // has more than 1 argument
                    char query_string1[1024];
                    strcpy(query_string1, query_string.c_str());
                    char* token = strtok(query_string1, "&");
                    while(token != NULL){
                        string token_string(token);
                        int temp_index = token_string.find("=");
                        if(temp_index == string::npos){
                            write(client_request.req_accept_id, "Bad Request Error: Retry!", 25);
                            close(client_request.req_accept_id);
                        } else{
                            client_request.reqBody[token_string.substr(0, temp_index)] = token_string.substr(temp_index + 1, -1);
                        }
                        token = strtok(NULL, "&");
                    }
                    cout<<"requestBody: "<<client_request.reqBody<<endl;
                }
            }   
        }

        // 
        int first_slash_index = client_request.req_filename.find_first_of("/", 0);
        int sec_slash_index = client_request.req_filename.find_first_of("/", first_slash_index + 1);

        if(sec_slash_index >= 0) {
            client_request.root_check = false;
        } else {
            client_request.root_check = true;
        }

        client_request.req_filename = root_dir + client_request.req_filename;
        changeDirectory(client_request);
    }
}

void Parser::changeDirectory(ClientRequest client_request){
    int next = client_request.req_filename.find_first_of("~", 0);
    int size = client_request.req_filename.size();
    if(next > 0 && next < size){
        int index = client_request.req_filename.find_first_of("/", next);
        string request = client_request.req_filename.substr(next + 1, index - (next + 1));
        string rest_str = client_request.req_filename.substr(index, size - index);
        client_request.req_filename.erase(next, size);
        client_request.req_filename = client_request.req_filename + request + "" + rest_str;
    }

    checkRequest(client_request);
}

bool Parser::checkFileExists(string fileName) {
    struct stat checker;
    if(stat(fileName.c_str(), &checker) != -1)
        return true;
    return false;
}

void Parser::requestQueue(ClientRequest client_request) {
    // Main thread will put the client's request to the request queue
    pthread_mutex_lock(&req_queue_lock);
    client_list.push_back(client_request);
    pthread_cond_signal(&req_queue_cond);
    pthread_mutex_unlock(&req_queue_lock);
}

void Parser::popRequest(){
    sleep(sleep_time);
    while(true) {
        ClientRequest client_request;
        transform(schedule_type.begin(), schedule_type.end(), schedule_type.begin(), ::toupper);    // convert schedule_type string to all uppercase

        // Check scheduling type
        if(schedule_type == "SJF"){     // SJF: Shortest Job First
            pthread_mutex_lock(&req_queue_lock);

            while(client_list.empty()){     // no client
                pthread_cond_wait(&req_queue_cond, &req_queue_lock);
            }
            client_list.sort(sortRequest);      // sort the job based on file size
            client_request =  client_list.front();
            client_list.pop_front();
            pthread_mutex_unlock(&req_queue_lock);
        }
        else if(schedule_type == "FCFS") {  // FCFs: First Come First Serve
            pthread_mutex_lock(&req_queue_lock);

            while(client_list.empty()){     // no client
                pthread_cond_wait(&req_queue_cond, &req_queue_lock);
            }

            client_request =  client_list.front();
            client_list.pop_front();
            pthread_mutex_unlock(&req_queue_lock);
        }

        pthread_mutex_lock(&print_lock);
        req_list.push_back(client_request);
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

        ClientRequest client_request;
        client_request = req_list.front();
        req_list.pop_front();

        time_t temp_time = time(NULL);
        tm* cur_time = gmtime(&temp_time);
        char cur_time_formatted[50];
        if(strftime(cur_time_formatted, 50, "%x:%X", cur_time) == 0){
            perror("time format error: ");        
        }
        client_request.req_serve_time = string(cur_time_formatted);
        pthread_mutex_unlock(&print_lock);
        
        // Send Data
        DataUtils d;
        d.sendData(client_request);
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

void print_client_request(ClientRequest client_request){
    cout<<"req_method: "<<client_request.req_method<<endl;
    cout<<"req_type: "<<client_request.req_type<<endl;
    cout<<"req_accept_id: "<<client_request.req_accept_id<<endl;
    cout<<"req_ip: "<<client_request.req_ip<<endl;
    cout<<"req_port_num: "<<client_request.req_port_num<<endl;
    cout<<"req_first_line: "<<client_request.req_first_line<<endl;
    cout<<"req_filename: "<<client_request.req_filename<<endl;
    cout<<"req_time: "<<client_request.req_time<<endl;
    cout<<"req_serve_time: "<<client_request.req_serve_time<<endl;
    cout<<"req_ctype: "<<client_request.req_ctype<<endl;
    cout<<"req_file_size: "<<client_request.req_file_size<<endl;
    cout<<"status_code: "<<client_request.status_code<<endl;
    cout<<"root_check: "<<client_request.root_check<<endl;
}

void Parser::checkRequest(ClientRequest client_request){
    int last_slash_index = client_request.req_filename.find_last_of("/");
    int temp = client_request.req_filename.find_first_of(".", last_slash_index + 1);

    cout<<"Checking if filename is REST API or not\n";
    if(temp == string::npos){       // filename is REST API
        cout<<"This is REST API\n";
        client_request.req_file_size = 0;
        client_request.status_file = false;
        client_request.is_rest_api = true;
        client_request.req_filename = client_request.req_filename.substr(last_slash_index+1,-1);

        requestQueue(client_request);
    } else {        // filename includes a file with extension
        client_request.is_rest_api = false;
        // Filename is a file with extension;
        if(checkFileExists(client_request.req_filename)){
            if(client_request.req_method == "GET"){
                ifstream file;
                file.open(client_request.req_filename);
                
                // Set req_file_size
                if(file.is_open()){
                    file.seekg(0, ios::end);
                    client_request.req_file_size = (int) file.tellg();
                } else {
                    client_request.req_file_size = 0;
                }

                client_request.status_file = true;
                file.close();
                requestQueue(client_request);
            }
        } else {
            write(client_request.req_accept_id, "Error 404: File Not Found", 25);
            client_request.status_code = 404;
            close(client_request.req_accept_id);
        }
    }
}