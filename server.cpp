#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
using namespace std;
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

class fileserver
{
    int n;
    char filename[200];
public:

void fetchfile(int clientsock) {
    bzero(filename, 200);
    n = read(clientsock, filename, 200);
    if (n < 0) {
        cerr << "Error reading file name";
        exit(1);
    }
    cout << strlen(filename) << endl;
    filename[strcspn(filename, "\r\n")] = '\0';
    int len = strlen(filename);

    cout << "Requested file: [" << filename << "]" << endl;
    cout << strlen(filename) << endl;
}

    void sendfile(int clientsock)
    {
        ifstream file(filename, ios::binary | ios::ate);
        if (!file)
        {
            string msg = "ERROR";
            send(clientsock, msg.c_str(), msg.size(), 0);
            cerr << "file not found" << endl;
            exit(1);
        }
        long filesize = file.tellg();
        file.seekg(0, ios::beg);

        send(clientsock, &filesize, sizeof(filesize), 0);
    
        char buffer[1024];
        while (!file.eof())
        {
            file.read(buffer, sizeof(buffer));
            send(clientsock, buffer, file.gcount(), 0);
        }
        file.close();
        cout << "file sent: " << filesize << " bytes";
    }
};
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "port no. not provided, program terminated";
        exit(1);
    }
    int sockfd, newsockfd, portno, n;
    char buffer[255];

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket.");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Binding failed");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("Error on Accept");

    fileserver serverobj;
    serverobj.fetchfile(newsockfd);
    serverobj.sendfile(newsockfd);
    close(newsockfd);
    close(sockfd);
    return 0;
}