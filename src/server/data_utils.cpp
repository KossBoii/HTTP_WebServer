#include "data_utils.h"
#include <unistd.h>
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
#include<algorithm>
#include <stdio.h>
#include "diamondhttp.h"

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

void DataUtils::sendData(ClientInfo client_info){
    if(client_info.status_file){
        if(client_info.req_type == "GET"){
            // Get current time 
            time_t temp_time = time(NULL);
            tm* cur_time = gmtime(&temp_time);
            char cur_time_formatted[50];
            if(strftime(cur_time_formatted, 50, "%x:%X", cur_time) == 0){
                perror("time format error: ");        
            }


            char last_mod_time[50];
            struct stat buf;
            stat(client_info.req_filename.c_str(), &buf);
            strcpy(last_mod_time, ctime(&buf.st_mtime));

            int temp = client_info.req_filename.find_last_of(".");
            string extension = client_info.req_filename.substr(temp+1, client_info.req_filename.size());

            // Check extension type
            if(extension == "txt" || extension == "html" || extension == "htm"){
                client_info.req_ctype = "text/html";
            }
            else if(extension == "gif" || extension == "jpeg" || extension == "jpg"){
                client_info.req_ctype = "image/" + extension;
            } else {
                client_info.req_ctype = " ";
            }

            stringstream ss;
            ss << client_info.req_file_size;
            string file_size = ss.str();
            string header = client_info.req_type + " " + client_info.req_method + " 200 OK\r\nDate: ";
            string temp_str(cur_time_formatted);
            header += temp_str;
            header = header + "\r\n" + "Server: diamondhttp 1.0\r\n" + "Last-Modified:";
            string temp_str2(last_mod_time);
            header += temp_str2;
            header += "Content-Type:" + client_info.req_ctype + "\r\n" + "Content-Length:" + file_size + "\r\n\r\n";

            if(send(client_info.req_accept_id, header.c_str(), header.length(), 0) == -1){
                perror("send: ");
            }

            ifstream file;
            char* content;
            size_t size;
            file.open(client_info.req_filename);
            if(file.is_open()){
                string read;
                file.seekg(0, ios::end);
                size = file.tellg();
                content = new char[size];
                file.seekg(0, ios::beg);
                file.read(content, size);
            } else {
                cout<<"Can't open file "<<client_info.req_filename<<endl;
            }

            if(send(client_info.req_accept_id, content, size, 0) == -1){
                perror("send");
            }
            file.close();
            client_info.status_code = 200;
            delete [] content;
        }
    } else {
        if(client_info.root_check){
            write(client_info.req_accept_id, "Error 404: File Not Found", 25);
        } 
        client_info.status_code = 404;

        listDir(client_info);
    }

    close(client_info.req_accept_id);
}

// List the directory
void DataUtils::listDir(ClientInfo client_info)
{
	struct dirent *de=NULL;
	DIR *d=NULL;
	int last = client_info.req_filename.find_last_of("/");
	string dir = client_info.req_filename.substr(0,last);
	vector<string> dirlist;
	char * dirname = new char[dir.size() + 1];
	std::copy(dir.begin(), dir.end(), dirname);
	dirname[dir.size()] = '\0';
	d=opendir(dirname);
	if(d == NULL) {
		//	perror("Couldn't open directory");
		write(client_info.req_accept_id,"Error 404: Directory Not Found",30);
		client_info.status_code = 404;
	} else {
		while(de = readdir(d)) {
			string s(de->d_name);
			dirlist.push_back(s);
		}

		vector<string>::iterator it;
		sort(dirlist.begin(),dirlist.end(), sortDirectory);
		write(client_info.req_accept_id,"Files Listing:",14);
		for ( it=dirlist.begin() ; it < dirlist.end(); it++ ) {
			write(client_info.req_accept_id, (*it).c_str(), strlen((*it).c_str()));
			write(client_info.req_accept_id,"\n",1);			
		}

		closedir(d);
		
		delete [] dirname;
		dirname = NULL;
	}
}