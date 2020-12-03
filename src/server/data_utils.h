#ifndef DATA_UTILS_H_
#define DATA_UTILS_H_
#include <sys/types.h>
#include <string.h>
#include <fstream>
#include "parser.h"

using namespace std;

class DataUtils {
public:
    void sendData(ClientInfo client_info);

private:
    void listDir(ClientInfo client_info);
};


#endif  // DATA_UTILS_H_