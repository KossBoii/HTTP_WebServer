#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

void welcomeMessage();
void postScreen();
void viewScreen();
// void clearConsole();
void viewSpecificQuestion(string title);

int main() {
    welcomeMessage();
    return 0;
}

void welcomeMessage() {
    string userInputStr;
    int userInput = 0;
    while(userInput != 3) {
        // clearConsole();
        cout << "                Welcome to the Question & Answer Forum                \n"
                "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
                "What would you like to do?\n"
                "\t1. Post a new Question\n\t2. View Questions\n\t3. Exit\n"
                "Enter corresponding number: ";
        cin >> userInputStr;
        try {
            userInput = stoi(userInputStr);
        } catch (exception &e) {
            cout << "\nInvalid input, only enter numbers 1, 2 or 3.\n\n\n";
            // clearConsole();
            welcomeMessage();
        }

        if (userInput == 1) {
            // clearConsole();
            postScreen();
        } else if (userInput == 2) {
            // clearConsole();
            viewScreen();
        } else if (userInput == 3) {
            // clearConsole();
            cout << "Goodbye..";
        } else {
            cout << "\nPlease enter either 1, 2 or 3 only.\n\n";
            welcomeMessage();
        }
    }
}


void postScreen(){
    string userInputStr, title, question;
    int userInput = 0;
    cout << "                                 Post                                 \n"
            "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
            "\nEnter -1 to exit\nEnter Question Title:\t";
    cin >> title;
    try {
        userInput = stoi(title);
        if(userInput == -1)
            return;
    } catch (exception ) {
    }
    cout << "\nEnter -1 to exit\nEnter Question: ";
    cin >> question;
    try {
        userInput = stoi(title);
        if(userInput == -1)
            return;
    } catch (exception ) {
    }

    // POST HERE
    //post(title + "," + question);

    /*SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    string addr = "localhost";
    if (0 == connect(s, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)))
        cout << "Connected.\n";
    else
        cout << "Connection Error.\n";

     */
    cout << "Question \"" << title << "\" has been posted.\n";
}

void viewScreen(){
    string userInputStr, title, question;
    string questionString;
    vector<string> questions;

    // GET here
    //questions = get();
    questionString = "Question1_Title,Question1:Question2_Title,Question2:Question3_Title,Question3:Question4_Title,Question4";

    stringstream ss(questionString);
    while (ss.good()) {
        string substr;
        getline(ss, substr, ':');
        questions.push_back(substr);
    }

    int userInput = 0;
    while(userInput != questions.size()+1) {
        // clearConsole();
        cout << "                              Questions                               \n"
                "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

        int counter = 1;
        for (int i = 0; i < questions.size(); i++) {
            cout << (i + 1) << ": ";

            int pos;
            pos = questions[i].find(",");
                cout << questions[i].substr(0, pos) << "\n\t\t";
                //questions[i].substr(pos + 1);
                cout << questions[i].substr(pos + 1) << "\n";

        }
        cout << "------------------------\n" << questions.size()+1 << ": Exit\n\n";

        cout << "Enter corresponding number: ";
        cin >> userInput;
        if(userInput > 0 && userInput < (questions.size()+1)){
            int pos;
            string title;
            pos = questions[userInput-1].find(",");
            title = questions[userInput-1].substr(0, pos);
            viewSpecificQuestion(title);
        }
        if(userInput > questions.size()+1)
            userInput = questions.size()+1;
    }

}

void viewSpecificQuestion(string title){
    int userInput;
    string question,answer,response;
    vector<string> answers;

    // Get question/answers by title

    response = "QuestionX:Answer1,Answer2,Answer3,Answer4,Answer5,Answer6,Answer7";
    int pos = response.find(':');
    question = response.substr(0, pos);
    answer = response.erase(0, pos+1);

    stringstream ss(answer);
    while (ss.good()) {
        string substr;
        getline(ss, substr, ',');
        answers.push_back(substr);
    }

    // clearConsole();
    cout << title << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    cout << "Q: " << question << "\n\n";
    for (int i = 0; i < answers.size(); i++) {
        cout << "A" << (i + 1) << ": " << answers[i] << "\n\n";
    }
    cout << "------------------------\n" <<
    answers.size()+1 << ": Submit New Answer\n" << answers.size()+2 << ": Exit\n\n";

    cout << "Enter corresponding number: ";
    cin >> userInput;

    if(userInput > 0 && userInput <= answers.size()){

    }else if(userInput == answers.size()+1) {
        string newAnswer;
        cout << "Enter new answer:\n\t";
        cin >> newAnswer;
        // POST HERE
        //post(title + "," + question);
    }
}


// Helper Methods
// void clearConsole(){
//     //system("pause");
//     for (int i = 0; i < 25; ++i) {
//         cout << "\n";
//     }
// }