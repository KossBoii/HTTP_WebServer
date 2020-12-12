#ifndef ANSWER_H_
#define ANSWER_H_
#include <iostream>
#include "helper_funcs.h"

using namespace std;

int addAnswer(Json::Value requestBody){
    string checkStr[3] = {"username", "id", "content"};
    bool foundAll = jsonContains(requestBody, checkStr, 3);

    if(!foundAll){
        return 400;
    } else{
        string username = requestBody["username"].asString();
        string content = requestBody["content"].asString();
        int question_id = stoi(requestBody["id"].asString());

        string ans_folder = root_dir + "/questions/" + 
            question_id_to_fname[question_id];

        if(!isDirExist(ans_folder))      // question_id does not EXIST    
            return 400;         // Bad Request
        
        string ans_path = ans_folder + "/answers.txt";   
        if(!isFileExist(ans_path)){      // answers does not EXIST yet 
            return 500;
        } else {
            ifstream input_file(ans_path.c_str());
            // Read info.txt 
            input_file.seekg(0, ios::end);
            streampos length = input_file.tellg();
            input_file.seekg(0, ios::beg);

            char buf[1024];
            input_file.read(buf, length);
            string temp(buf);
            input_file.close();

            Json::Value val;
            Json::Reader reader;
            if(reader.parse(temp, val)){
                Json::Value writeMe;
                for (const auto& key : requestBody.getMemberNames()) {
                    if(key != "id"){
                        writeMe[key] = requestBody[key];
                    }
                }
                val["answers"].append(writeMe);
                
                // Write to answers.txt
                ofstream w_file;
                w_file.open(ans_path.c_str(), ios::trunc);
                Json::StyledWriter styledWriter;
                w_file<<styledWriter.write(val);
                w_file.close();
                
                return 200;
            } else{                     // cant parse Json Object from answers.txt
                return 500;             // Server Internal Error
            }
        }
    }

    // Code should never go down and return over here
    return 500;
}

// int removeAnswer(Json::Value requestBody){

//     // Code should never go down and return over here
//     return 500;
// }

// int updateAnswer(Json::Value requestBody){

//     // Code should never go down and return over here
//     return 500;
// }

int getAnswers(Json::Value requestBody, Json::Value& result){
    string checkStr[1] = {"id"};
    bool foundAll = jsonContains(requestBody, checkStr, 1);

    if(!foundAll){
        return 400;
    } else{
        int question_id = stoi(requestBody["id"].asString());
        string ans_folder = root_dir + "/questions/" + 
            question_id_to_fname[question_id];

        if(!isDirExist(ans_folder)){      // question_id does not EXIST    
            return 400;         // Bad Request
        }
        
        string ans_path = ans_folder + "/answers.txt";   
        if(!isFileExist(ans_path)){      // answers does not EXIST yet 
            return 500;
        } else {
            ifstream input_file(ans_path.c_str());
            // Read info.txt 
            input_file.seekg(0, ios::end);
            streampos length = input_file.tellg();
            input_file.seekg(0, ios::beg);

            char buf[1024];
            input_file.read(buf, length);
            string temp(buf);
            input_file.close();

            Json::Value val;
            Json::Reader reader;
            if(reader.parse(temp, val)){
                result = val;
                return 200;
            } else{                     // cant parse Json Object from answers.txt
                return 500;             // Server Internal Error
            }
        }
    }

    // Code should never go down and return over here
    return 500;
}



#endif  // ANSWER_H_