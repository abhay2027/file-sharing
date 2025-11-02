
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <netdb.h>
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
class fileclient
{
public:
    void recievefile(int sock)
    {
        long filesize;
        int n = recv(sock, &filesize, sizeof(filesize), 0);
        if (n < 0)
        {
            cerr << "failed to recieve file size";
            exit(1);
        }
        if (strncmp((char *)&filesize, "ERROR", 5) == 0)
        {
            cerr << "file not found";
        }

        ofstream outfile("the_metamorphosis.pdf",ios::binary);
        if (!outfile){
            cerr<<"failed to open output file";
        }
        char buffer[1024];
        long totalrecieved=0;
        while(totalrecieved<filesize){
            n=recv(sock,buffer,sizeof(buffer),0);
            if(n<0){
                cerr<<"file not recived";
                exit(1);
            }
            outfile.write(buffer,n);
            totalrecieved+=n;
        }
        outfile.close();
        cout<<"file recieved "<<totalrecieved;
    }
};
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    if (argc < 3)
    {
        cout << "stderr" << argv[0] << "hostname port" << endl;
        exit(1);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("error opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("connection failed");

    fileclient clientobj;
    clientobj.recievefile(sockfd);
    close(sockfd);

    return 0;
}
