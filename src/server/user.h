#ifndef USER_H_
#define USER_H_
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include "parser.h"
#include "helper_funcs.h"

using namespace std;

int addUser(Json::Value requestBody){
    // Check arguments inside requestBody
    string checkStr[2] = {"username", "password"};
    bool foundAll = jsonContains(requestBody, checkStr, 2);

    if(!foundAll){          // should have username and password
        return 400;
    } else{
        // Create user with username
        string username = requestBody["username"].asString();
        string folder_name = root_dir + "/users/" + username;
        if(mkdir(folder_name.c_str(), 0777) != -1){
            // Overwrite total users
            if(!inc_total_users()){
                return 500;
            } else {
                cout<<"Successfully created user " + username<<endl;
                ofstream w_file;
                w_file.open((root_dir + "/users/" + username + "/info.txt").c_str());
                Json::StyledWriter styledWriter;
                w_file<<styledWriter.write(requestBody);
                w_file.close();
                return 201;         // Created
            }
        } else{
            cout<<"Cant created user " + username<<endl;
            return 400;         // Bad Request
        }
    }

    // Code should never go down and return over here
    return 500;
}

int removeUser(Json::Value requestBody){
    int status_code = user_check_Del_Update_Permission(requestBody);
    if(status_code != 0){
        return status_code;
    } else{         // Allow delete & update on user now!!
        string username = requestBody["username"].asString();
        string user_folder = root_dir + "/users/" + username;
        int status = removeDir(user_folder);
        if(status < 0){
            cout<<"removeUser: cant remove user "<<username<<endl;
            return 500;             // Server Internal Error
        } else {
            // Overwrite total users
            if(!dec_total_users()){
                return 500;
            } else {
                cout<<"removeUser: successfully remove user "<<username<<endl;
                return 204;         // Deleted
            }
        }
    }

    // Code should never go down and return over here
    return 500;
}

int updateUser(Json::Value requestBody){    
    int status_code = user_check_Del_Update_Permission(requestBody);
    if(status_code != 0){
        cout<<"not allowed to update\n";
        return status_code;
    } else{         // Allow delete & update on user now!!
        string username = requestBody["username"].asString();
        Json::Value value;
        int status = getUserContent(username, value);

        if(status != 200){
            return status;
        } else{
            // Insert updated value:
            Json::Value new_value;
            Json::StyledWriter styledWriter;
            bool has_new_password = false;

            for(auto const& key : requestBody.getMemberNames()){
                if(key == "password" && has_new_password)
                    continue;
                else if(key == "new_password"){
                    new_value["password"] = requestBody["new_password"];
                    has_new_password = true;
                } else{
                    new_value[key] = requestBody[key];
                }
            }

            update(requestBody, new_value);

            // Writeback to info.txt
            string user_folder = root_dir + "/users/" + username;
            ofstream w_file;
            w_file.open((user_folder + "/info.txt").c_str(), ios::trunc);  
            w_file<<styledWriter.write(new_value);
            w_file.close();
            return 200;
        }
    }

    // Code should never go down and return over here
    return 500;
}

int getUser(Json::Value requestBody, Json::Value& result){
    // Check arguments inside requestBody
    string checkStr1[2] = {"username", "password"};
    bool foundAll = jsonContains(requestBody, checkStr1, 2);

    if(!foundAll){          // Does not have specific key for this METHOD
        return 400;
    } else{
        // Update the user from the database
        string username = requestBody["username"].asString();
        string password = requestBody["password"].asString();
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
            // Check arguments inside requestBody
            string checkStr2[2] = {"username", "password"};
            foundAll = jsonContains(value, checkStr2, 2);

            if(foundAll){
                string user_username = value["username"].asString();
                string user_password = value["password"].asString();

                if(username != user_username){      // username DIFFERENT username in info.txt
                    return 500;         // Server Internal Error
                } else{
                    if(password == user_password){
                        // Only allow getting user if given password 
                        // the same as the password stored in the database
                        result = value;
                        return 200;
                    } else{             // password is DIFFERENT => cant delete
                        cout<<"Password is DIFFERENT\n";
                        return 400;     // Bad Request
                    }
                }
            } else{             // cant find either username or password in info.txt
                return 500;
            }
        } else{                  // cant parse Json Object from info.txt
            return 500;         // Server Internal Error
        }
    }

    // Code should never go down and return over here
    return 500;
}

#endif  // USER_H_