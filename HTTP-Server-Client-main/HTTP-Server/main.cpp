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
struct arg_struct {int client_socket;long long* timer;};
vector <arg_struct*> Clients_Structs;



/*
this function is used to parse String of command and divide it if it is separted by space
*/
string Edits(const string& str){
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
        return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}




/*
this function is used to split request as if command is give client_get index.html it pasrse the command line by space
*/
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




/*
this function if File is already Exist or not
*/
bool CheckExistenceFile(string fileName){
    ifstream ifile;
    ifile.open(fileName);
    if(ifile)
        return true;
    else
        return false;
}




/*
this function is used to send information of file to socket
*/
void ChunkSend(int socket , string s) {
    const char *beginner = s.c_str();
    int i=0;
    while(i < s.length())
    {
        send(socket, beginner + i,min(500, (int)s.length() - i), 0);
        i+=500;
    }
}



/*
this function is used to close connection when its time is out
*/
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
                cout << "A client connection closed" << endl;
            }
            i++;
        }
        sleep(1);
    }
}




/*
this function is used to save file;
*/
void FileSave (string fileName, string content){
    ofstream f_stream(fileName.c_str());
    f_stream.write(content.c_str(), content.length());
}




/*
 this function get Type of file which is either html or txt  
*/
string ContentType(string fileName){

     if (fileName.find(".html") != std::string::npos)
        return "text/html";
    
    if (fileName.find(".txt") != std::string::npos)
        return "text/plain";
   
}



/*
this function is used to read all content of file and save it in string Content 
*/
string RequestedFileContent(string fileName){
    string line;
    string content = "";
    ifstream myfile;
    myfile.open(fileName);
    while(getline(myfile, line))
        content += line;
    return content;
}




/*
this function is used to get all content of file  
*/
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




/*
the server take the requests and parse it and take filename and its type and printits content
*/
void GetFileINformation(string fileName,int socketClient)
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




/*
the server recive file from client and the response will be 200
*/
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

//return the file with given name content in string
string fileContent(string name)
{
    string req;
    string c = "";
    ifstream f;
    f.open(name);
    while(getline(f, req))
        c += req;
    return c;
}
/*
Server trys to get file if it exists response will be 200 else it will be 404 
*/
void Client_Get( string fileName,int socketClient)
{
                // cout << "filename:   " << "fileName";
                if (CheckExistenceFile(fileName)){
                string ok = "HTTP/1.1 200 OK\\r\\n \n";
                char * tab2 = &ok[0];
                write(socketClient, tab2, strlen(tab2));
                string content = fileContent(fileName);
                // cout << "\t" << content <<endl;
                int fileSize = content.size();
                write(socketClient, &fileSize, sizeof(int));
                ChunkSend(socketClient, content);
                } else {
                string str = "HTTP/1.1 404 Not Found\\r\\n \n";
                // cout<< "\thhh" << endl;
                char * tab2 = &str[0];
                write(socketClient, tab2, strlen(tab2));
                string content = GetFileContent("not_found.txt");
                int fileSize = content.size();
                write(socketClient, &fileSize, sizeof(int));
                ChunkSend(socketClient, content);
                }
}


/*
parse command line and check whether it is Post or get
*/
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
            GetFileINformation(fileName,socketClient);
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



/*

After creating socket we use bind as the bind connect between the socket and serecer_address and portNum
and listen is used to make server listen to requests come to it;
and then we use accept to execute this requests;
*/

int main(int argc, char const *argv[]){
    if (argc != 2){printf("Not Valid Arguments or Missing Port");exit(EXIT_FAILURE);}
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int portNum;
    struct sockaddr_in Newaddress;
    portNum = atoi(argv[1]);
    Newaddress.sin_family = AF_INET;
    Newaddress.sin_addr.s_addr = INADDR_ANY;
    Newaddress.sin_port = htons(portNum);
    memset(Newaddress.sin_zero, '\0', sizeof Newaddress.sin_zero);
    int addresslength = sizeof(Newaddress);
    if (serverSocket < 0){printf("error creating Server Socket");exit(EXIT_FAILURE);}
    int bind_status = bind(serverSocket, (struct sockaddr *)&Newaddress, sizeof(Newaddress));
    if (bind_status < 0){printf("error binding server");exit(EXIT_FAILURE);}
    int listen_status = listen(serverSocket, 10) ;
    if (listen_status < 0){printf("error in listening");exit(EXIT_FAILURE);}
    pthread_t timeout;
    int socket;
    pthread_create(&timeout, NULL, TimeOutOFClient, NULL);
    while(1){
        cout << "Connecting!!!" << endl;
        cout << flush;
        if ((socket = accept(serverSocket, (struct sockaddr *)&Newaddress, (socklen_t*)&addresslength)) < 0){
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







