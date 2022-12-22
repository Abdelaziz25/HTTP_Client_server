#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>

#define BUFFER_SIZE 1024
#define IP_ADDRESS "192.168.1.59"

using namespace std;
//Return vector of all the requests in the file with the name givem
vector<string> findReq(string name)
{
    string req;
    ifstream f;
    vector<string> v;
    f.open(name);
    while(getline(f, req))
        v.push_back(req);
    return v;
}

vector<string> splitor(string str)
{
    vector<string> v;
    int count = 0;
    string word = "";
    for(int i =0; i < str.length(); i++)
    {
        if (str[i] == ' ')
        {
            v.push_back(word);
            word = "";
            count++;
            if (count == 2)
            { 
                break;
            }
        }
        else 
        {
            if (str[i] != '/')
            {
                 word = word + str[i];
            }
        }
    }
    return v;
}
//return the file with given name content in string
string getPostFileContent(string name)
{
    string req;
    string c = "";
    ifstream f;
    f.open(name);
    while(getline(f, req))
        c += req;
    return c;
}

void saveFile (string name, string content)
{
    ofstream f_stream(name.c_str());
    f_stream.write(content.c_str(), content.length());
}



int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        cout << "Enter required inputs" << endl;
        return 1;
    }
    //Initilize socket creation
    int portNumber = atoi(argv[2]);
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE] = {0};
    //ipv6 tcp
    int socAddr = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddr, '0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNumber);
    //Creation failure
    if (socAddr < 0)
    {
        cout << "Socket creation error" << endl;
        return 2;
    }
    if(inet_pton(AF_INET, IP_ADDRESS, &serverAddr.sin_addr) <= 0)
    {
        cout << "Address not found" << endl;
        return 3;
    }
    // Fail to connect to server
    if (connect(socAddr, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "Connection Failed" << endl;
        return 4;
    }
    //getting vector of all the requests in the file
    vector<string> req = findReq("requests.txt");
    for (int i = 0; i < req.size() ; i++)
    {
        string request = req[i];
        vector<string> reqSplits = splitor(request);
        bzero(buffer, BUFFER_SIZE);
        string type = reqSplits[0];
        char * tempBuffer = &request[0];
        cout << request << endl;
        //Send and read response from socket
        send(socAddr, tempBuffer, strlen(tempBuffer), 0);
        //response result
        read(socAddr, buffer, BUFFER_SIZE);
        cout << buffer << endl;
        if (reqSplits[0] == "get" || reqSplits[0] == "client_get")
        {
            int size;
            //getting file size
            read(socAddr, &size, sizeof(int));
            // cout << "\t bbbbbbbb valrea"<<endl;
            cout << "String Size : " << size << endl;
            //It is used to save file content coming from server
            string content = "";
            // cout << "\t ho valrea"<<endl;
            while (true)
            {
                char contentBuffer[BUFFER_SIZE];
                //reaching end of file
                if (content.size() == size)
                {
                    break;
                }
                // cout << "\tbefore valrea"<<endl;
                long valRead = read(socAddr, contentBuffer, BUFFER_SIZE);
                // cout << "\tbefore valrea"<<endl;
                // cout << "\t" << valRead;
                if (valRead <= 0)
                {
                    cout << "File Completed";
                    break;
                }
                // cout<< "Akromvic" <<endl;
                content += string(contentBuffer,valRead);
            }
            // cout <<endl<< "fgkjgbg"<<endl;
            cout << content << endl;
            saveFile(reqSplits[1] , content);
        } 
        else if (reqSplits[0] == "post" || reqSplits[0] == "client_post")
        {
            //put the file content in string
            string fileContent = getPostFileContent(reqSplits[1]);
            char * tempFile = &fileContent[0];
            bzero(buffer, BUFFER_SIZE);
            cout << fileContent << endl;
            //Sending the file to server
            send(socAddr, tempFile, strlen(tempFile), 0);
            //getting response from server
            read(socAddr, buffer, BUFFER_SIZE);
            cout << buffer << endl;
        }
    }

    // while (1){
    //     fgets(buffer, BUFFER_SIZE,stdin);
    //     string req = buffer;
    //     send(socAddr, buffer, strlen(buffer), 0 );
    //     cout << "Buffer Sent" << endl;
    //     cout << flush;
    //     bzero(buffer, BUFFER_SIZE);
    //     read(socAddr, buffer, BUFFER_SIZE);
    //     cout << buffer << endl;
    //     cout << flush;
    //     bzero(buffer, BUFFER_SIZE);
    //     vector<string> spl = splitor(req);
    //     if (spl[0] == "post" || spl[0] == "client_post"){
    //         string fileContent = getPostFileContent(spl[1]);
    //         char * tempFile = &fileContent[0];
    //         cout << fileContent << endl;
    //         cout << flush;
    //         send(socAddr, tempFile, strlen(tempFile), 0 );
    //         read(socAddr, buffer, BUFFER_SIZE);
    //         cout << buffer << endl;
    //         cout << flush;
    //         bzero(buffer, BUFFER_SIZE);
    //     } else if (spl[0] == "get" || spl[0] == "client_get"){
    //         int size;
    //         read(socAddr, &size, sizeof(int));
    //         cout << "String Size : " << size << endl;
    //         cout << flush;
    //         string content = "";
    //         while (true){
    //             char contentBuffer [BUFFER_SIZE];
    //             if (content.size() == size){
    //                 break;
    //             }
    //             long valRead = read(socAddr, contentBuffer, BUFFER_SIZE);
    //             if (valRead <= 0){
    //                 cout << "File Completed";
    //                 cout << flush;
    //                 break;
    //             }
    //             content += string(contentBuffer,valRead);
    //         }
    //         cout << content << endl;
    //         saveFile(spl[1] , content);
    //     }
    // }
    return 0;
}

