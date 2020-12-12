#ifndef DIAMONDHTTP_H_
#define DIAMONDHTTP_H_
#include <iostream>
#include <string>
#include <pthread.h>
#include "server.h"
#include "parser.h"
#include <map>

using namespace std;

//------------------------ Declare global variables for HTTP Web Server ------------------------ 

// pthread lock & cond
extern pthread_mutex_t req_queue_lock;
extern pthread_cond_t req_queue_cond;
extern pthread_mutex_t print_lock;
extern pthread_cond_t print_cond;

class Server;
extern Parser* parser;
extern Server* run;

extern int sock_id;
extern string port;
extern string schedule_type;
extern int num_thread;
extern bool help_manual;
extern int sleep_time;
extern string log_file;
extern string root_dir;

extern map<string, int> rest_api_list;
extern map<int, string> status_res_list;
extern map<int, string> question_id_to_fname;



#endif  // DIAMONDHTTP_H_
