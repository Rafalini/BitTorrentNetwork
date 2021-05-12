#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>

#include "Functions.hpp"

#define PORT 8080
#define ADRESS "172.17.0.2"

int main(int argc, char const *argv[])
{

    std::string message;
    if(argc<2)
      message = "Message from client to server";
    else
      message = std::string(argv[1]);

    std::vector<std::string> arguments(argv + 2, argv + argc);
    message = getJson(arguments,message);

    struct sockaddr_in serv_addr;
    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout<<"Socket creation error"<<std::endl;
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, ADRESS, &serv_addr.sin_addr)<=0)
    {
        std::cout<<"Invalid address/ Address not supported"<<std::endl;
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout<<"Connection Failed"<<std::endl;
        exit(EXIT_FAILURE);
    }

    send(sock, message.c_str(), message.length(), 0 );
    std::string response(1024, 0);
    std::cout<<"sent:    "<<message<<std::endl;
    read( sock, &response[0], 1024);
    std::cout<<"recived: "<<response<<std::endl;
    return 0;
}
