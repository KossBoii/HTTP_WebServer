#include "diamondhttp.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "server.h"
#include "parser.h"

using namespace std;

// Init Global Variables
pthread_mutex_t req_queue_lock;
pthread_cond_t req_queue_cond;
pthread_mutex_t print_lock;
pthread_cond_t print_cond;
pthread_t thread_scheduler;
pthread_t thread_pool[30];

string port = "8080";
string schedule_type = "FCFS";
int num_thread = 20;
bool help_manual = false;
int sleep_time = 30;
string log_file = "log.txt";
string root_dir =  "./../database";
int sock_id = 0;

Parser* parser = new Parser();           // Only create 1 instance of Parser object per server run
Server* run = new Server();         // Only create 1 instance of Server object per server run
sig_atomic_t flag = 0;

map<string, int> rest_api_list = {
    {"addQuestion", 0},
    {"updateQuestion", 1},
    {"removeQuestion", 2},
    {"getQuestion", 3},
    {"getQuestions", 4},
    {"addAnswer", 5},
    {"getAnswers", 6},
    {"addUser", 7},
    {"updateUser", 8},
    {"removeUser", 9},
    {"getUser", 10}
};

map<int, string> status_res_list = {
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {204, "No Content"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"}
};

map<int, string> question_id_to_fname = map<int, string>();

void print_help() {
    cout<<"--------------------------------------------------------------------"<<endl;
    cout<<"NAME"<<endl;
    cout<<"\t./http_server - run the HTTP server built from scratch"<<endl;
    cout<<"AUTHORS"<<endl;
    cout<<"\tLong Truong, Isaiah Britto, and Hari Kifle"<<endl;
    cout<<"DESCRIPTION"<<endl;
    cout<<"\tExtra flags to modify HTTP server\n\n";
    cout<<"\t-h\n";
    cout<<"\t\tdisplay help manual for ./http_server\n\n";
    cout<<"\t-p port_num\n";
    cout<<"\t\tchange server's port to port_num (By default, server's port is 8080) Ex: -p 80\n\n";
    cout<<"\t-s sched_type\n";
    cout<<"\t\tchange thread scheduling type to sched_type (By default, scheduling type is FCFS) Ex: -s SJF\n\n";
    cout<<"Use Ctrl+C to terminate the HTTP server at any time";
    cout<<"--------------------------------------------------------------------"<<endl;
}

void signal_handler(int signal) {
    flag++;
    close(sock_id);
    delete parser;
    parser = NULL;
    delete run;
    run = NULL;
    pthread_exit(NULL);
    exit(signal);
}

int main(int argc, char** argv) {
    fstream iFile;
    iFile.open((root_dir + "/config.txt").c_str(), ios::in);
    if(iFile.is_open()){
        string line;
        while(getline(iFile, line)){
            int index = line.find_first_of("->");
            int key = stoi(line.substr(0, index));
            string val = line.substr(index + 2, -1);
            question_id_to_fname.insert({key, val});
        }
        iFile.close();
    }

    int opt = getopt(argc, argv, "hp:s:");
    while(opt != -1){
        switch(opt){
            case 'h':
                help_manual = true;
                break;
            case 'p':
                port = optarg;
                break;
            case 's':
                schedule_type = optarg;
                break;
            default:
                cout<<"Unknown flag "<<opt<<endl;
                break;
        }

        opt = getopt(argc, argv, "hp:s:");
    }

    if(help_manual){    // If help manual flag turns on ==> display help manual
        print_help();
    } else {
        if(pthread_mutex_init(&req_queue_lock, NULL) != 0){
            cout<<"error: init mutex lock failed\n";
            return 1;
        }

        pthread_mutex_init(&print_lock, NULL);
        pthread_cond_init(&req_queue_cond, NULL);
        pthread_cond_init(&print_cond, NULL);
        pthread_create(&thread_scheduler, NULL, &Parser::popRequest_helper, parser);

        for(int i=0; i<num_thread; i++){
            pthread_create(&thread_pool[i], NULL, &Parser::processRequest_helper, parser);
        }

        run->accept_connection();

        // Destroy & Release mutex lock & cond
        pthread_mutex_destroy(&req_queue_lock);
        pthread_cond_destroy(&req_queue_cond);
        pthread_mutex_destroy(&print_lock);
        pthread_cond_destroy(&print_cond);
        pthread_exit(NULL);
    }

    signal(SIGINT, signal_handler);
    close(sock_id);
    delete parser;
    parser = NULL;
    delete run;
    run = NULL;
    return 0;
}