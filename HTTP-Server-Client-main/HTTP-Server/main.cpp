#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <string>
#include <bits/stdc++.h>

using namespace std;

// string OK_RESPOND = "HTTP/1.1 200 OK\\r\\n \n";
// string NOT_FOUND_RESPOND = "HTTP/1.1 404 Not Found\\r\\n \n";
// void * clientConnection(void* P_socketClient);
// void * clientTimeOut (void *);
// void sendChunks(int socket , string s);
// string readFileContent(string photoName);
// string getContentType(string fname);
// string getRequestedFileContent(string fileName);
// void saveFile (string fileName, string content);
// bool checkFileExist(string fileName);
// string trim(const string& str);
// vector<string> splitRequest(string str);
struct arg_struct {int client_socket;long long* timer;};
vector <arg_struct*> Clients_Structs;

string Edits(const string& str){
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
        return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

vector<string> RequestSplit(string str){
    str = Edits(str);
    vector<string> vec;
    int counter = 0;
    string word = "";
    for (auto x : str){
        if (x == ' '){
            vec.push_back(word);
            word = "";
            counter = counter+1;
            if (counter == 2) break;
        }
        else
            word = word + x;
    }
    return vec;
}

bool CheckExistenceFile(string fileName){
    ifstream ifile;
    ifile.open(fileName);
    if(ifile)
        return true;
    else
        return false;
}
void ChunkSend(int socket , string s) {
    const char *beginner = s.c_str();
    int i=0;
    while(i < s.length())
    {
        send(socket, beginner + i,min(500, (int)s.length() - i), 0);
        i+=500;
    }
    // for (int i = 0; i < s.length(); i += 500)
    //     send(socket, beginner + i,min(500, (int)s.length() - i), 0);
}

void * TimeOutOFClient (void *){
    while (1){

        int i=0;
        while(i < Clients_Structs.size())
        {
            long long interval = clock() - *(Clients_Structs[i]->timer);
            if (interval >= 1e4){
                close((Clients_Structs[i]->client_socket));
                Clients_Structs.erase(Clients_Structs.begin()+i);
                i--;
                cout << "A Client Connection Is Closed" << endl;
            }
            i++;
        }

        // for (int i = 0; i < Clients_Structs.size() ; i++){
        //     long long interval = clock() - *(Clients_Structs[i]->timer);
        //     if (interval >= 1e4){
        //         close((Clients_Structs[i]->client_socket));
        //         Clients_Structs.erase(Clients_Structs.begin()+i);
        //         i--;
        //         cout << "A Client Connection Is Closed" << endl;
        //     }
        // }
        sleep(1);
    }
}

void FileSave (string fileName, string content){
    ofstream f_stream(fileName.c_str());
    f_stream.write(content.c_str(), content.length());
}

string ContentType(string fileName){

     if (fileName.find(".html") != std::string::npos)
        return "text/html";
    
    if (fileName.find(".txt") != std::string::npos)
        return "text/plain";
   
}

string RequestedFileContent(string fileName){
    string line;
    string content = "";
    ifstream myfile;
    myfile.open(fileName);
    while(getline(myfile, line))
        content += line;
    return content;
}

string GetFileContent(string fileName){
    ifstream fin(fileName);
    size_t buffer_size = 1024;
    char buffer[buffer_size];
    size_t len = 0;
    ostringstream streamer;
    while((len = fin.readsome(buffer, buffer_size)) > 0)
        streamer.write(buffer, len);
    return streamer.str();
}

void Get(string fileName,int socketClient)
{
            string real = "";
            // for (int i = 1; i < fileName.size() ; i++){real += fileName[i];}
            int i =1 ;
            while( i < fileName.size())
            {
                real += fileName[i];
                i++;
            }
            if (CheckExistenceFile(real)){
                cout << endl << real << endl;
                string fContent =  RequestedFileContent(real);
                int cLength = fContent.size();
                string conType = ContentType(real);
                string str = "HTTP/1.1 200 OK\nContent-Type: " + conType+ "\nContent-Length: " + to_string(cLength) + "\n\n" + RequestedFileContent(real);
                char * tab2 = &str[0];
                write(socketClient, tab2, strlen(tab2));
            } else {
                string fContent =  RequestedFileContent("not_found.txt");
                int cLength = fContent.size();
                string conType = ContentType(fileName);
                string str = "HTTP/1.1 404 Not Found\n";
                char * tab2 = &str[0];
                write(socketClient, tab2, strlen(tab2));
            }
}
int Client_Post(string fileName,int socketClient,char buffer[4096])
{
  string str = "HTTP/1.1 200 OK\\r\\n \n";
            char * tab2 = &str[0];
            write(socketClient, tab2, strlen(tab2));
            bzero(buffer, 4096);
            long long val2 = read(socketClient, buffer, 4096);
            if (val2 <= 0) return 1;
            cout << buffer << endl;
            cout << flush;
            string fileToSave = string(buffer);
            FileSave(fileName, fileToSave);
            string res = "File is saved successfully \n";
            char * msg = &res[0];
            write(socketClient, msg, strlen(msg));
            return 2;
}
void Client_Get( string fileName,int socketClient)
{
   if (CheckExistenceFile(fileName)){
                string ok = "HTTP/1.1 200 OK\\r\\n \n";
                char * tab2 = &ok[0];
                write(socketClient, tab2, strlen(tab2));
                string content = GetFileContent(fileName);
                int fileSize = content.size();
                write(socketClient, &fileSize, sizeof(int));
                ChunkSend(socketClient, content);
            } else {
                string str = "HTTP/1.1 404 Not Found\\r\\n \n";
                char * tab2 = &str[0];
                write(socketClient, tab2, strlen(tab2));
                string content = GetFileContent("not_found.txt");
                int fileSize = content.size();
                write(socketClient, &fileSize, sizeof(int));
                ChunkSend(socketClient, content);
            }
}
void * ConnectionOfClient(void* socket_Client)
{
    string total_buffer = "";
    arg_struct client_Struct = *((arg_struct*) socket_Client);
    int socketClient = client_Struct.client_socket;
    long long* currentClock = client_Struct.timer;
    *(currentClock) = clock();
    while (1){
        *(currentClock) = clock();
        char buffer[4096] = {0};
        long long val = read(socketClient, buffer, 4096);
        if (val <= 0) break;
        cout << buffer << endl;
        cout << flush;
        buffer[strlen(buffer) - 1] = '\0';
        total_buffer += buffer;
        vector<string> Split = RequestSplit(total_buffer);

        if (Split[0] == "get" || Split[0] == "client_get") {
            string fileName = Split[1];
            Client_Get(fileName,socketClient);

        } else if (Split[0] == "post" || Split[0] == "client_post"){
            string fileName = Split[1];
           int x = Client_Post(fileName,socketClient,buffer);
           if(x==1)
           {
             break;
           }
        } else if (Split[0] == "GET") {
            string fileName = Split[1];
            Get(fileName,socketClient);
        } else if (Split[0] == "close") {
            break;
        } else {
            string str = "HTTP/1.1 404 Not Found\\r\\n \n";
            char * tab2 = new char [str.length()+1];
            strcpy (tab2, str.c_str());
            write(socketClient, tab2, strlen(tab2));
        }
        bzero(buffer, 4096);
        total_buffer = "";
    }
    close(socketClient);
    return NULL;
}

int main(int argc, char const *argv[]){
    if (argc != 2){printf("Not Valid Arguments or Missing Port");exit(EXIT_FAILURE);}
    int server_address;
    server_address = socket(AF_INET, SOCK_STREAM, 0);
    int portNum;
    struct sockaddr_in Newaddress;
    portNum = atoi(argv[1]);
    Newaddress.sin_family = AF_INET;
    Newaddress.sin_addr.s_addr = INADDR_ANY;
    Newaddress.sin_port = htons(portNum);
    memset(Newaddress.sin_zero, '\0', sizeof Newaddress.sin_zero);
    int addresslength = sizeof(Newaddress);
    if (server_address < 0){printf("error creating Server Socket");exit(EXIT_FAILURE);}
    int bind_status = bind(server_address, (struct sockaddr *)&Newaddress, sizeof(Newaddress));
    if (bind_status < 0){printf("error binding server");exit(EXIT_FAILURE);}
    int listen_status = listen(server_address, 10) ;
    if (listen_status < 0){printf("error in listening");exit(EXIT_FAILURE);}
    pthread_t timeout;
    int socket;
    pthread_create(&timeout, NULL, TimeOutOFClient, NULL);
    while(1){
        cout << "Connecting!!!" << endl;
        cout << flush;
        if ((socket = accept(server_address, (struct sockaddr *)&Newaddress, (socklen_t*)&addresslength)) < 0){
            printf("error in connection");
            exit(EXIT_FAILURE);
        }
        pthread_t t;
        long long myClock = 0;
        arg_struct argues;
        argues.timer = &myClock;
        argues.client_socket = socket;
        Clients_Structs.push_back(&argues);
        pthread_create(&t, NULL, ConnectionOfClient, &argues);
    }
    return 0;
}







