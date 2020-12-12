#ifndef HELPER_FUNCS_H_
#define HELPER_FUNCS_H_
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <sys/stat.h>
#include <jsoncpp/json/json.h>
#include "diamondhttp.h"

using namespace std;

// Helper Functions for total_questions and total_users
int get_total_questions(){
    ifstream file;
    file.open((root_dir + "/questions/total_questions.txt").c_str());
    if(file){
        string line;
        getline(file, line);
        file.close();  
        return stoi(line);
    }
    return -1;          // cant open total_questions.txt
}
bool inc_total_questions(){
    int cur_quests = get_total_questions();
    if(cur_quests != -1){
        ofstream w_file;
        w_file.open((root_dir + "/questions/total_questions.txt").c_str(), ios::trunc);
        w_file<<(cur_quests + 1);
        w_file.close();
        return true;
    }
    return false;           // cant open total_questions.txt
}
bool dec_total_questions(){
    int cur_quests = get_total_questions();
    if(cur_quests == 0){
        return false;       // cant decrease anymore
    } else if(cur_quests == -1){
        return false;       // cant open total_questions.txt
    }

    ofstream w_file;
    w_file.open((root_dir + "/questions/total_questions.txt").c_str(), ios::trunc);
    w_file<<(cur_quests - 1);
    w_file.close();
    return true;
}
int get_total_users(){
    ifstream file;
    file.open((root_dir + "/users/total_users.txt").c_str());
    if(file){
        string line;
        getline(file, line);
        file.close();  
        return stoi(line);
    }
    return -1;          // cant open total_questions.txt
}
bool inc_total_users(){
    int cur_users = get_total_questions();
    if(cur_users != -1){
        ofstream w_file;
        w_file.open((root_dir + "/users/total_users.txt").c_str(), ios::trunc);
        w_file<<(cur_users + 1);
        w_file.close();
        return true;
    }
    return false;           // cant open total_questions.txt
}
bool dec_total_users(){
    int cur_users = get_total_questions();
    if(cur_users == 0){
        return false;       // cant decrease anymore
    } else if(cur_users == -1){
        return false;       // cant open total_questions.txt
    }

    ofstream w_file;
    w_file.open((root_dir + "/users/total_users.txt").c_str(), ios::trunc);
    w_file<<(cur_users - 1);
    w_file.close();
    return true;
}

int get_total_answers(int question_id){
    ifstream file;
    string folder = root_dir + "/questions/" +
        question_id_to_fname[question_id] + "/total_answers.txt";
    file.open(folder.c_str());
    if(file){
        string line;
        getline(file, line);
        file.close();  
        return stoi(line);
    }
    return -1;          // cant open total_questions.txt
}


// HTTP Request Helper Functions
int removeDir(const string path){
    string folderPath = path[path.length() - 1] != '/' ? path : path.substr(0, path.length()-1);
    DIR* folder = opendir(folderPath.c_str());
    if(folder){
        // opened the directory
        struct dirent* nextFile;
        while((nextFile = readdir(folder)) != NULL){
            string filePath;

            if(strcmp(nextFile->d_name, ".") != 0 && strcmp(nextFile->d_name, "..") != 0){
                filePath = folderPath + "/" + nextFile->d_name;
                if(nextFile->d_type == DT_REG){             // Check if it is a file
                    if(remove(filePath.c_str()) == 0){

                    }
                    else
                        return -1;
                }
                else if(nextFile->d_type == DT_DIR){        // Check if it is a directory
                    // Remove recursively folders inside the root folder
                    removeDir(filePath);
                }
            }
        }
        closedir(folder);

        // Remove the empty folder at the end
        if(rmdir(folderPath.c_str()) == 0){
            return 0;
        }
        else{
            // This means the folder is still not really empty
            return -1;
        }
    }
    else{
        perror(("rmdir: " + folderPath).c_str());
        return -1;
    }
}

bool isDirExist(const string& dir_path){
    DIR* folder = opendir(dir_path.c_str());
    if(folder){             // directory exists and has opened it
        closedir(folder);
        return true;
    }
    return false;
}

bool isFileExist(const string& file_name){
    return access(file_name.c_str(), 0) == 0;
}

bool jsonContains(Json::Value requestBody, string arr[], const int& arr_len){
    bool foundStr[arr_len];
    bool foundAll = false;

    for(int i=0; i<arr_len; i++){
        foundStr[i] = false;
    }

    for(auto const& key : requestBody.getMemberNames()){
        for(int i=0; i<arr_len; i++){
            if(key == arr[i]){
                foundStr[i] = true;
            }
        }
    }

    for(int i=0; i<arr_len; i++){
        if(!foundStr[i]){
            return false;
        }
    }
    return true;
}

void update(Json::Value& dest, Json::Value& src) {
    if (!dest.isObject() || !src.isObject()) return;

    for (const auto& key : src.getMemberNames()) {
        if (dest[key].isObject()) {
            update(dest[key], src[key]);
        } else {
            dest[key] = src[key];
        }
    }
}

bool writeJson(string path, Json::Value writeMe){
    ofstream w_file;
    w_file.open(path.c_str());
    if(w_file){             // successfully open the file
        Json::StyledWriter styledWriter;
        w_file<<styledWriter.write(writeMe);
        w_file.close();
        return true;
    }
    return false;           // cant open the file
}

int checkUserValid(string username, string password){
    string user_folder = root_dir + "/users/" + username;
    // Check if username EXISTS and info.txt EXISTS or not
    if(!isDirExist(user_folder))          // username does not EXIST
        return 400;         // Bad Request

    if(!isFileExist(user_folder + "/info.txt"))          // somehow info.txt got deleted
        return 500;         // Server Internal Error

    // Check if the username and password given matches before removing user
    ifstream input_file((user_folder + "/info.txt").c_str());

    // Read info.txt 
    input_file.seekg(0, ios::end);
    streampos length = input_file.tellg();
    input_file.seekg(0, ios::beg);

    char buf[1024];
    input_file.read(buf, length);
    string temp(buf);

    // Parse username and password from info.txt
    Json::Reader reader;  
    Json::Value value;
    if(reader.parse(temp, value)){
        string checkStr2[2] = {"username", "password"};
        bool foundAll = jsonContains(value, checkStr2, 2);

        if(!foundAll){
            return 500;             // Server Internal Error
        } else {
            string user_username = value["username"].asString();
            string user_password = value["password"].asString();

            if(username != user_username){      // username DIFFERENT username in info.txt
                return 500;         // Server Internal Error
            } else{
                if(password == user_password){
                    return 0;       // True
                } else{ 
                    return 400;     // Bad Request     
                }
            }
        }
    } else{                     // cant parse Json Object from info.txt
        return 500;             // Server Internal Error
    }
    return 500;                 // Server Internal Error
}

int getQuestionContent(int question_id, Json::Value& result){
    // Checking question_id
    if(question_id < 0){
        return 400;
    }

    int cur_total_ques = get_total_questions();
    if(cur_total_ques == -1){
        return 500;
    } else if(question_id > cur_total_ques){
        return 400;
    }

    string ques_folder = root_dir + "/questions/" + question_id_to_fname[question_id];
    // Check if question EXISTS and info.txt EXISTS or not
    if(!isDirExist(ques_folder))          // question does not EXIST
        return 400;         // Bad Request

    if(!isFileExist(ques_folder + "/info.txt"))          // somehow info.txt got deleted
        return 500;         // Server Internal Error

    ifstream input_file((ques_folder + "/info.txt").c_str());

    // Read info.txt 
    input_file.seekg(0, ios::end);
    streampos length = input_file.tellg();
    input_file.seekg(0, ios::beg);

    char buf[1024];
    input_file.read(buf, length);
    string temp(buf);

    Json::Reader reader; 
    if(reader.parse(temp, result)){
        // Check arguments inside requestBody
        string checkStr[4] = {"username", "title", "content"};
        bool foundAll = jsonContains(result, checkStr, 3);

        if(foundAll){
            return 200;
        } else{             // cant find either title or content in info.txt
            return 500;
        }
    } else{                     // cant parse Json Object from info.txt            
        return 500;             // Server Internal Error
    }

    // Code should never go down and return over here
    return 500;
}

int getUserContent(string username, Json::Value& result){
    string user_folder = root_dir + "/users/" + username;
    
    // Check if username EXISTS and info.txt EXISTS or not
    if(!isDirExist(user_folder))          // username does not EXIST
        return 400;         // Bad Request

    if(!isFileExist(user_folder + "/info.txt"))          // somehow info.txt got deleted
        return 500;         // Server Internal Error

    // Check if the username and password given matches before removing user
    ifstream input_file((user_folder + "/info.txt").c_str());

    // Read info.txt 
    input_file.seekg(0, ios::end);
    streampos length = input_file.tellg();
    input_file.seekg(0, ios::beg);

    char buf[1024];
    input_file.read(buf, length);
    string temp(buf);

    // Parse username and password from info.txt
    Json::Reader reader;
    if(reader.parse(temp, result)){
        string checkStr[2] = {"username", "password"};
        bool foundAll = jsonContains(result, checkStr, 2);

        if(!foundAll){
            return 500;         // Server Internal Error
        } else{
            string user_username = result["username"].asString();
            if(username != user_username){      // username DIFFERENT username in info.txt
                return 500;         // Server Internal Error
            } else{
                return 200;
            }
        }
    }
    else{                   // cant parse Json Object from info.txt
        return 500;         // Server Internal Error
    }

    // Code should never go down and return over here
    return 500;
}

int check_Del_Update_Permission(Json::Value requestBody){
    // Check arguments inside requestBody
    string checkStr[3] = {"id", "username", "password"};
    bool foundAll = jsonContains(requestBody, checkStr, 3);
    
    if(!foundAll){
        return 400;
    } else {
        int question_id = stoi(requestBody["id"].asString());
        // Checking question_id
        if(question_id < 0){
            return 400;
        }

        //-----------------------------------------------------------------------------------------
        //                          Getting current total questions
        //-----------------------------------------------------------------------------------------
        int cur_total_ques = get_total_questions();
        if(cur_total_ques == -1){
            return 500;
        }

        //-----------------------------------------------------------------------------------------
        //                          Check validity of question_id
        //-----------------------------------------------------------------------------------------
        if(question_id > cur_total_ques){          // invalid index
            return 400;
        } else{
            string username = requestBody["username"].asString();
            string password = requestBody["password"].asString();

            //-----------------------------------------------------------------------------------------
            //                          Check validity of username
            //-----------------------------------------------------------------------------------------
            int userValid = checkUserValid(username, password);
            if(userValid != 0){
                return userValid;
            } else{                 // user existed in the database
                //-----------------------------------------------------------------------------------------
                //             Matching given username with question's owner
                //-----------------------------------------------------------------------------------------
                Json::Value value1;
                int status = getQuestionContent(question_id, value1);
                if(status != 200){
                    return status;
                } else {
                    string question_owner = value1["username"].asString();
                    if(username != question_owner){
                        return 400;         // Bad Request
                    } else{
                        // given user in reqBody created this question
                        // ==> allowed to update/delete the question
                        return 0;
                    }
                }
            }
        }
    }

    // Code should never go down and return over here
    return 500;
}

int user_check_Del_Update_Permission(Json::Value requestBody){
    // Check arguments inside requestBody
    string checkStr[2] = {"username", "password"};
    bool foundAll = jsonContains(requestBody, checkStr, 2);

    if(!foundAll){          // Does not have specific key for this METHOD
        return 400;         // Bad Request
    } else{
        // Remove the user from the system
        string username = requestBody["username"].asString();
        string password = requestBody["password"].asString();
        Json::Value content;
        int status_code = getUserContent(username, content);
        if(status_code != 200){
            return status_code;
        } else{
            string user_password = content["password"].asString();
            
            if(password != user_password){
                return 400;     // Bad Request
            } else{
                // Only allow remove/update user if given password 
                // the same as the password stored in the database
                return 0;
            }
        }
    }
    
    // Code should never go down and return over here
    return 500;
}








#endif  // HELPER_FUNCS_H_