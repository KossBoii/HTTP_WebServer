#ifndef DIAMONDHTTP_H_
#define DIAMONDHTTP_H_
#include <iostream>
#include <string>
#include <pthread.h>
#include "server.h"
#include "parser.h"

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



#endif  // DIAMONDHTTP_H_
