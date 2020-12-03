#ifndef PARSER_H_
#define PARSER_H_
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <list>

using namespace std;

struct ClientIdentity {
    int accept_id;
    string ip;
    int port_num;
    string req_time;
};

struct ClientInfo {
    string req_method;
    string req_type;
    string req_version;

    int req_accept_id;
    string req_ip;
    u_int16_t req_port_num;

    string req_first_line;    
    string req_filename;
    string req_time;
    string req_serve_time;

    string req_ctype;
    int req_file_size;
    bool status_file;
    int status_code;
    bool root_check;
};

class Parser {
public:
    // Class Methods
    void parseRequest(ClientIdentity client);
    void requestQueue(ClientInfo client_info);

    void checkRequest(ClientInfo client_info);
    void popRequest();
    void processRequest();

    static void* popRequest_helper(void* client);
    static void* processRequest_helper(void* client);
    
private:
    bool checkFileExists(string fileName);
    void changeDirectory(ClientInfo client_info);

    // Class Variables
    list<ClientInfo> client_list;
    list<ClientInfo> req_list;
};


#endif  // PARSER_H_