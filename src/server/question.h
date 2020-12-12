#ifndef QUESTION_H_
#define QUESTION_H_
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include "parser.h"
#include "helper_funcs.h"

using namespace std;

int addQuestion(Json::Value requestBody){
    // Check arguments inside requestBody
    string checkStr[3] = {"title", "content", "username"};
    bool foundAll = jsonContains(requestBody, checkStr, 3);

    if(!foundAll){          // should have title, content and username
        return 400;
    } else{
        string username = requestBody["username"].asString();
        string title = requestBody["title"].asString();
        string content = requestBody["content"].asString();
        string user_folder = root_dir + "/users/" + username;

        if(!isDirExist(user_folder))      // username does not EXIST    
            return 400;         // Bad Request
            
        // Gets the title and content
        // string quest_name = "question" + to_string(cur_total_ques + 1);
        string quest_name = username + title + content;
        hash<string> str_hash;
        string quest_folder = root_dir + "/questions/" + 
            to_string(str_hash(quest_name));
        if(mkdir(quest_folder.c_str(), 0777) != -1){
            // requestBody["id"] = to_string(cur_total_ques + 1);
            // requestBody["owner"] = requestBody["username"].asString();
            bool checkWrite = writeJson(quest_folder + "/info.txt", requestBody);
            if(!checkWrite){
                return 500;         // Server Internal Issue
            }

            // Overwrite total questions
            if(!inc_total_questions()){
                return 500;
            } else{
                int id = get_total_questions();
                cout<<"Successfully created question " + id<<endl;
                question_id_to_fname.insert({id, to_string(str_hash(quest_name))});
                to_string(str_hash(quest_name));

                ofstream w_file;
                w_file.open((quest_folder + "/answers.txt").c_str(), ios::out);
                Json::Value writeMe;
                writeMe["answers"] = Json::arrayValue;
                Json::StyledWriter styledWriter;
                w_file<<styledWriter.write(writeMe);
                w_file.close();

                // // Add questions to question_list in username
                // Json::Value user;
                // int status = getUserContent(username, user);
                // user["question_list"].append(id);


                // ofstream w_file;
                // w_file.open((root_dir + "/users/" + username + "/info.txt").c_str());
                // Json::StyledWriter styledWriter;
                // w_file<<styledWriter.write(user);
                // w_file.close();

                return 201;
            }
        } else{
            cout<<"Cant created question_name " + quest_name<<endl;
            return 400;         // Bad Request
        }
    }

    // Code should never go down and return over here
    return 500;
}

int removeQuestion(Json::Value requestBody) {
    int perm_code = check_Del_Update_Permission(requestBody);
    if(perm_code != 0){
        return perm_code;
    } else{
        int question_id = stoi(requestBody["id"].asString());
        string username = requestBody["username"].asString();
        // given user in reqBody created this question
        // ==> allowed to delete the question
        string ques_folder = root_dir + "/questions/" + question_id_to_fname[question_id];
        int remove_code = removeDir(ques_folder);
        if(remove_code < 0){
            cout<<"removeUser: cant remove question"<<to_string(question_id)<<endl;
            return 500;             // Server Internal Error
        } else {
            // Overwrite total questions
            if(!dec_total_questions()){
                return 500;
            } else{
                cout<<"removeUser: successfully remove question"<<to_string(question_id)<<endl;
                question_id_to_fname.erase(question_id);

                int prev_total_questions = get_total_questions() + 1;

                // Fix question_id in the map
                for(int i=question_id + 1; i<=prev_total_questions; i++){
                    auto entry = question_id_to_fname.find(i);
                    if(entry != end(question_id_to_fname)){
                        auto const value = move(entry->second);
                        question_id_to_fname.erase(entry);
                        question_id_to_fname.insert({i - 1, move(value)});
                    }
                }
                
                // // Fix question_id in username folder
                // Json::Value user;
                // int status = getUserContent(username, user);
                // auto list = user["question_list"];

                // for(int i=0; i < list.size(); i++){
                //     if(list[i] > question_id){
                //         list[i] = list[i].asInt() - 1;
                //     }
                // }
                // user["question_list"] = list;
                
                // // Update to user
                // ofstream w_file;
                // w_file.open((root_dir + "/users/" + username + "/info.txt").c_str());
                // Json::StyledWriter styledWriter;
                // w_file<<styledWriter.write(user);
                // w_file.close();


                return 204;
            }
        }
    }
    // Code should never go down and return over here
    return 500;
}

int updateQuestion(Json::Value requestBody){
    int perm_code = check_Del_Update_Permission(requestBody);

    if(perm_code != 0){
        return perm_code;
    } else{
        // given user in reqBody created this question
        // ==> allowed to update the question
        Json::Value result;
        int ques_id = stoi(requestBody["id"].asString());
        int code = getQuestionContent(ques_id, result);

        if(code != 200){
            return code;
        } else {
            Json::Value newVal;
            for(auto const& key : requestBody.getMemberNames()){
                if(key != "password" && key != "id"){
                    newVal[key] = requestBody[key];
                }
            }

            update(result, requestBody);
            writeJson(root_dir + "/questions/" + question_id_to_fname[ques_id] + "/info.txt", result);
            return 200;
        }
    }
    
    // Code should never go down and return over here
    return 500;
}

int getQuestion(Json::Value requestBody, Json::Value& result){
    // Check arguments inside requestBody
    string checkStr[1] = {"id"};
    bool foundAll = jsonContains(requestBody, checkStr, 1);

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
        if(question_id > cur_total_ques){          // Wrong index
            return 400;
        } else{
            int status_code = getQuestionContent(question_id, result);
            return status_code;
        }
    }

    // Code should never go down and return over here
    return 500;
}

int getQuestions(Json::Value& result){
    int cur_total_ques = get_total_questions();
    if(cur_total_ques == -1){
        return 500;
    }

    bool interrrupted = false;
    for(int i = 1; i <= cur_total_ques; i++){
        Json::Value temp;
        Json::Value reqBody;
        reqBody["id"]=i;
        int status_code = getQuestion(reqBody, temp);

        if(status_code != 200){
            interrrupted = true;
            break;
        }
        result[to_string(i)] = temp;
    }

    if(interrrupted){
        return 500;
    }
    return 200; 
}

#endif  // QUESTION_H_