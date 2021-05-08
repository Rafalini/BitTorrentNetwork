#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#define PORT 8080
#define ADRESS "172.17.0.2"

int main(int argc, char const *argv[])
{
  char responseBuffer[1024] = {0};
  std::string message;
    if(argc<2)
      message = "Message from client to server";
    else
      message = std::string(argv[1]);

    struct sockaddr_in serv_addr;
    int sock = 0, valread;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout<<"Socket creation error"<<std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, ADRESS, &serv_addr.sin_addr)<=0)
    {
        std::cout<<"Invalid address/ Address not supported"<<std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout<<"Connection Failed"<<std::endl;
        return -1;
    }
    
    send(sock, message.c_str(), message.length(), 0 );

    std::cout<<"sent:    "<<message<<std::endl;
    valread = read( sock, responseBuffer, 1024);
    std::cout<<"recived: "<<responseBuffer<<std::endl;
    return 0;
}
