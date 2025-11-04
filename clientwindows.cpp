#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

void error(const char *msg)
{
    cerr << msg << " (" << WSAGetLastError() << ")" << endl;
    WSACleanup();
    exit(1);
}

class fetch
{
protected:
    int n;
    char filename[200];
    char menu[500];

public:
    void getfilename(SOCKET sock)
    {
        n = recv(sock, menu, sizeof(menu), 0);
        if (n <= 0)
        {
            error("Failed to read menu");
        }
        menu[n] = '\0';
        cout << menu;

        cin.getline(filename, 200);
        send(sock, filename, strlen(filename), 0);
        cout << "Requested file: " << filename << endl;
    }
};

class recvfile : public fetch
{
public:
    void filerecv(SOCKET sock)
    {
        long filesize;
        int n = recv(sock, (char *)&filesize, sizeof(filesize), 0);
        if (n <= 0)
        {
            error("Failed to receive file size");
        }

        // Check if server sent an error message instead of filesize
        if (strncmp((char *)&filesize, "ERROR", 5) == 0)
        {
            cerr << "File not found on server." << endl;
            return;
        }

        ofstream outfile(filename, ios::binary);
        if (!outfile)
        {
            cerr << "Failed to open output file for writing." << endl;
            return;
        }

        char buffer[1024];
        long totalReceived = 0;
        while (totalReceived < filesize)
        {
            n = recv(sock, buffer, sizeof(buffer), 0);
            if (n <= 0)
            {
                cerr << "File transfer interrupted." << endl;
                break;
            }
            outfile.write(buffer, n);
            totalReceived += n;
        }

        outfile.close();
        cout << "File received successfully (" << totalReceived << " bytes)" << endl;
    }
};

class server
{
    int argc;
    char **argv;

public:
    server(int c, char *v[])
    {
        argc = c;
        argv = v;
    }

    void ser()
    {
        WSADATA wsa;
        SOCKET sock;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        int portno;

        if (argc < 3)
        {
            cerr << "Usage: " << argv[0] << " <hostname> <port>" << endl;
            exit(1);
        }

        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            error("Failed to initialize Winsock");
        }

        portno = atoi(argv[2]);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
        {
            error("Error opening socket");
        }

        server = gethostbyname(argv[1]);
        if (server == NULL)
        {
            cerr << "Error: no such host." << endl;
            WSACleanup();
            exit(1);
        }

        memset((char *)&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
        serv_addr.sin_port = htons(portno);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            error("Connection failed");
        }

        recvfile receivefile;
        receivefile.getfilename(sock);
        receivefile.filerecv(sock);

        closesocket(sock);
        WSACleanup();
    }
};

int main(int argc, char *argv[])
{
    server s(argc, argv);
    s.ser();
    return 0;
}

