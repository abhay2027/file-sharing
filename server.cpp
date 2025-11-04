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
    string menu;

public:
    void fetchfile(int clientsock)
    {
        menu =
            "Available files are:\n"
            "1. Alice in Borderland S03E03 HIN ENG JAP 1080p.mkv\n"
            "2. C++ Programming Task PDF.pdf\n"
            "3. The Metamorphosis.pdf\n"
            "4. white night.pdf\n"
            "Enter the name of file: ";

        write(clientsock, menu.c_str(), menu.size());
        bzero(filename, 200);
        n = read(clientsock, filename, 200);
        if (n <= 0)
        {
            cerr << "Error reading file name";
            return;
        }
        filename[strcspn(filename, "\r\n")] = '\0';
        cout << "Requested file: [" << filename << "]" << endl;
    }

    void sendfile(int clientsock)
    {
        ifstream file(filename, ios::binary | ios::ate);
        if (!file)
        {
            string msg = "ERROR";
            send(clientsock, msg.c_str(), msg.size(), 0);
            cerr << "file not found" << endl;
            return;
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
        cout << "file sent: " << filesize << " bytes" << endl;
    }
};
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "port no. not provided, program terminated";
        exit(1);
    }
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket.");
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Binding failed");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    fileserver serverobj;

    cout << "Server started on port " << portno << endl;

    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            perror("Error on Accept");
            continue;
        }
        cout << "Client connected" << endl;
        serverobj.fetchfile(newsockfd);
        serverobj.sendfile(newsockfd);
        cout << "Client disconnected" << endl;
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}

