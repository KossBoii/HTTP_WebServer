#ifndef PARSER_H_
#define PARSER_H_
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <list>
#include <jsoncpp/json/json.h>

using namespace std;

struct ClientIdentity {
    int accept_id;
    string ip;
    int port_num;
    string req_time;
};

struct ClientRequest {
    string req_first_line;      // first line of sent request IMPORTANT
    string req_method;
    string req_filename;
    string req_type;
    string req_version;
    Json::Value reqBody;
    bool is_rest_api;

    int req_accept_id;
    u_int16_t req_port_num;
    string req_ip;
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
    void requestQueue(ClientRequest client_request);

    void checkRequest(ClientRequest client_request);
    void popRequest();
    void processRequest();

    static void* popRequest_helper(void* client);
    static void* processRequest_helper(void* client);
    
private:
    bool checkFileExists(string fileName);
    void changeDirectory(ClientRequest client_request);

    // Class Variables
    list<ClientRequest> client_list;
    list<ClientRequest> req_list;
};


#endif  // PARSER_H_