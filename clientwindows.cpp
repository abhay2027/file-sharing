#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
using namespace std;

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

void error(const char *msg)
{
    cerr << msg << " (" << WSAGetLastError() << ")" << endl;
    exit(1);
}

class fileclient
{
    char filename[200];
    int n;
    char menu[500];

public:
    void files(SOCKET sock)
    {
        n = recv(sock, menu, 500, 0);
        if (n <= 0)
        {
            cerr << "Failed to receive menu" << endl;
            exit(1);
        }
        menu[n] = '\0';
        cout << menu;

        memset(filename, 0, sizeof(filename));
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\r\n")] = '\0';
        n = send(sock, filename, strlen(filename), 0);
        if (n <= 0)
        {
            cerr << "Failed to send filename" << endl;
            exit(1);
        }
        cout << filename << endl;
    }

    void recievefile(SOCKET sock)
    {
        long filesize;
        int n = recv(sock, (char *)&filesize, sizeof(filesize), 0);
        if (n <= 0)
        {
            cerr << "Failed to receive file size" << endl;
            exit(1);
        }

        if (strncmp((char *)&filesize, "ERROR", 5) == 0)
        {
            cerr << "File not found" << endl;
            return;
        }

        ofstream outfile(filename, ios::binary);
        if (!outfile)
        {
            cerr << "Failed to open output file" << endl;
            exit(1);
        }

        char buffer[1024];
        long totalReceived = 0;
        while (totalReceived < filesize)
        {
            n = recv(sock, buffer, sizeof(buffer), 0);
            if (n <= 0)
            {
                cerr << "File not fully received" << endl;
                break;
            }
            outfile.write(buffer, n);
            totalReceived += n;
        }
        outfile.close();
        cout << "File received: " << totalReceived << " bytes" << endl;
    }
};

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3)
    {
        cerr << "Usage: " << argv[0] << " <hostname> <port>" << endl;
        exit(1);
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        error("WSAStartup failed");
    }

    int portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
        error("Error opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        cerr << "Error: No such host" << endl;
        WSACleanup();
        exit(1);
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Connection failed");

    fileclient clientobj;
    clientobj.files(sockfd);
    clientobj.recievefile(sockfd);

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
