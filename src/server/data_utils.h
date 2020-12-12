#ifndef DATA_UTILS_H_
#define DATA_UTILS_H_
#include <sys/types.h>
#include <string.h>
#include <fstream>
#include "parser.h"
#include <jsoncpp/json/json.h>

using namespace std;

class DataUtils {
public:
    void sendData(ClientRequest client_request);

private:
    // HTTP Extra Functions
    string createReqHeader(ClientRequest client_request, int status_code, int content_size, string cur_time_formatted, string last_mod_time);
    
    // Directory-related Functions
    void listDir(ClientRequest client_request);
};


#endif  // DATA_UTILS_H_