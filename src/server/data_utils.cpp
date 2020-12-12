#include "data_utils.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <time.h>
#include <pthread.h>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <jsoncpp/json/json.h>
#include <stdio.h>
#include <map>
#include <vector>
#include "diamondhttp.h"
#include "user.h"
#include "question.h"
#include "answer.h"

using namespace std;

// Sort the list based on alphabets
bool sortDirectory(const string &lhs, const string &rhs)
{
   for( string::const_iterator lit = lhs.begin(), rit = rhs.begin(); lit != lhs.end() && rit != rhs.end(); ++lit, ++rit )
      if( tolower( *lit ) < tolower( *rit ) )
         return true;
      else if( tolower( *lit ) > tolower( *rit ) )
         return false;
   if( lhs.size() < rhs.size() )
      return true;
   return false;
}

void DataUtils::sendData(ClientRequest client_request){
    // cout<<"----------------------------------------------\n";
    // cout<<client_request.status_file<<endl;
    // cout<<"----------------------------------------------\n";

    if(!client_request.is_rest_api){
        if(client_request.req_method == "GET"){
            cout<<"This is GET request"<<endl;

            // Get current time 
            time_t temp_time = time(NULL);
            tm* cur_time = gmtime(&temp_time);
            char cur_time_formatted[50];
            if(strftime(cur_time_formatted, 50, "%x:%X", cur_time) == 0){
                perror("time format error: ");        
            }

            char last_mod_time[50];
            struct stat buf;
            stat(client_request.req_filename.c_str(), &buf);
            strcpy(last_mod_time, ctime(&buf.st_mtime));

            int temp = client_request.req_filename.find_last_of(".");
            if(temp != string::npos){       // no file extension
                string extension = client_request.req_filename.substr(temp+1, client_request.req_filename.size());

                // Check extension type
                if(extension == "txt" || extension == "html" || extension == "htm"){
                    client_request.req_ctype = "text/html";
                }
                else if(extension == "gif" || extension == "jpeg" || extension == "jpg"){
                    client_request.req_ctype = "image/" + extension;
                } else {
                    client_request.req_ctype = " ";
                }

                string temp_str(cur_time_formatted);
                string temp_str2(last_mod_time);
                string header = createReqHeader(client_request, 200, client_request.req_file_size, temp_str, temp_str2);

                if(send(client_request.req_accept_id, header.c_str(), header.length(), 0) == -1){
                    perror("send: ");
                }

                ifstream file;
                char* content;
                size_t size;
                file.open(client_request.req_filename);
                if(file.is_open()){
                    string read;
                    file.seekg(0, ios::end);
                    size = file.tellg();
                    content = new char[size];
                    file.seekg(0, ios::beg);
                    file.read(content, size);
                } else {
                    cout<<"Can't open file "<<client_request.req_filename<<endl;
                }

                if(send(client_request.req_accept_id, content, size, 0) == -1){
                    perror("send");
                }
                file.close();
                client_request.status_code = 200;
                delete [] content;
            }
        } 
    } else {        // filename is REST API
        // Get current time 
        time_t temp_time = time(NULL);
        tm* cur_time = gmtime(&temp_time);
        char cur_time_formatted[50];
        if(strftime(cur_time_formatted, 50, "%x:%X", cur_time) == 0){
            perror("time format error: ");        
        }

        char last_mod_time[50];
        struct stat buf;
        stat(client_request.req_filename.c_str(), &buf);
        strcpy(last_mod_time, ctime(&buf.st_mtime));

        // Execution
        map<string,int>::iterator i = rest_api_list.find(client_request.req_filename);
        string header;
        string temp_str(cur_time_formatted);
        string temp_str2(last_mod_time);
        int status_code = 0;
        if(i != rest_api_list.end()){
            Json::Value resultBody;
            switch(rest_api_list[client_request.req_filename]){
            case 0:         // addQuestion
                if(client_request.reqBody.empty() || client_request.req_method != "POST")         // request body is empty
                    status_code = 400;
                else
                    status_code = addQuestion(client_request.reqBody);
                break;
            case 1:         // updateQuestion
                if(client_request.reqBody.empty() || client_request.req_method != "PUT")         // request body is empty
                    status_code = 400;
                else
                    status_code = updateQuestion(client_request.reqBody);
                break;
            case 2:         // removeQuestion
                if(client_request.reqBody.empty() || client_request.req_method != "DELETE")         // request body is empty
                    status_code = 400;
                else
                    status_code = removeQuestion(client_request.reqBody);
                break;
            case 3:         // getQuestion
                if(client_request.reqBody.empty() || client_request.req_method != "GET")         // request body is empty
                    status_code = 400;
                else{
                    status_code = getQuestion(client_request.reqBody, resultBody);
                }
                break;
            case 4:         // getQuestions
                if(client_request.reqBody.empty() || client_request.req_method != "GET")         // request body is empty
                    status_code = getQuestions(resultBody);
                else
                    status_code = 400;                    
                break;
            case 5:         // addAnswer
                if(client_request.reqBody.empty() || client_request.req_method != "POST")         // request body is empty
                    status_code = 400;
                else
                    status_code = addAnswer(client_request.reqBody);
                break;
            case 6:         // getAnswers
                if(client_request.reqBody.empty() || client_request.req_method != "GET")         // request body is empty
                    status_code = 400; 
                else
                    status_code = getAnswers(client_request.reqBody, resultBody);
                break;
            case 7:         // addUser
                if(client_request.reqBody.empty() || client_request.req_method != "POST"){         // request body is empty
                    cout<<"reqBody: "<<client_request.reqBody<<endl;
                    cout<<"req_method: "<<client_request.req_method<<endl;
                    status_code = 400;
                }
                else{
                    cout<<"Executing addUser\n";
                    status_code = addUser(client_request.reqBody);
                }
                break;
            case 8:         // updateUser
                if(client_request.reqBody.empty() || client_request.req_method != "PUT")         // request body is empty
                    status_code = 400;
                else
                    status_code = updateUser(client_request.reqBody);
                break;
            case 9:         // removeUser
                if(client_request.reqBody.empty() || client_request.req_method != "DELETE"){         // request body is empty
                    status_code = 400;    
                }
                else
                    status_code = removeUser(client_request.reqBody);
                break;
            case 10:         // getUser
                if(client_request.reqBody.empty() || client_request.req_method != "GET")         // request body is empty
                    status_code = 400;
                else
                    status_code = getUser(client_request.reqBody, resultBody);
                break;
            }

            int size = 0;
            string content;
            if(status_code == 200 && !resultBody.empty()){
                client_request.req_ctype = "application/json";
                content = resultBody.toStyledString();
                size = content.length();
            }else{
                client_request.req_ctype = " ";
            }

            // Send request header
            string header = createReqHeader(client_request, status_code, size, temp_str, temp_str2);                
            cout<<"header: \n"<<header<<endl;
            if(send(client_request.req_accept_id, header.c_str(), header.length(), 0) == -1){
                perror("send: ");
            }
            
            // Send request body
            if(status_code == 200 && !resultBody.empty()){
                if(send(client_request.req_accept_id, content.c_str(), content.length(), 0) == -1){
                    perror("send");
                }
            }      
        } else {
            cout<<"This REST API was not defined\n";
            string header = createReqHeader(client_request, 501, 0, temp_str, temp_str2);
            if(send(client_request.req_accept_id, header.c_str(), header.length(), 0) == -1){
                perror("send: ");
            }
        }
    }

    close(client_request.req_accept_id);
}

string DataUtils::createReqHeader(ClientRequest client_request, int status_code, int content_size, string cur_time_formatted, string last_mod_time){
    string header = client_request.req_type + "/" + client_request.req_version + 
        " " + to_string(status_code) + " " + status_res_list[status_code] +"\r\n" + 
        "Date: " + cur_time_formatted + "\r\n" + "Server: diamondhttp 1.0\r\n" + "Last-Modified:" +
        last_mod_time + "Content-Type:" + client_request.req_ctype + "\r\n" + "Content-Length:" + 
        to_string(content_size) + "\r\n\r\n";

    return header;
}

// List the directory
void DataUtils::listDir(ClientRequest client_request)
{
	struct dirent *de=NULL;
	DIR *d=NULL;
	int last = client_request.req_filename.find_last_of("/");
	string dir = client_request.req_filename.substr(0,last);
	vector<string> dirlist;
	char * dirname = new char[dir.size() + 1];
	std::copy(dir.begin(), dir.end(), dirname);
	dirname[dir.size()] = '\0';
	d=opendir(dirname);
	if(d == NULL) {
		//	perror("Couldn't open directory");
		write(client_request.req_accept_id,"Error 404: Directory Not Found",30);
		client_request.status_code = 404;
	} else {
		while(de = readdir(d)) {
			string s(de->d_name);
			dirlist.push_back(s);
		}

		vector<string>::iterator it;
		sort(dirlist.begin(),dirlist.end(), sortDirectory);
		write(client_request.req_accept_id,"Files Listing:",14);
		for ( it=dirlist.begin() ; it < dirlist.end(); it++ ) {
			write(client_request.req_accept_id, (*it).c_str(), strlen((*it).c_str()));
			write(client_request.req_accept_id,"\n",1);			
		}

		closedir(d);
		
		delete [] dirname;
		dirname = NULL;
	}
}