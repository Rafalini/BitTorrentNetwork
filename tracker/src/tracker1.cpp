#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>

#include "Functions.hpp"

#define PORT 8080
#define IPPROTOCOL 0

namespace pt = boost::property_tree;

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTOCOL)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //binding
    checkError(bind(server_fd, (struct sockaddr *)&address, sizeof(address)), "bind failed");

    for(;;) //forever together love etc
    {
      std::string response(1024, 0);
      checkError(listen(server_fd, 3),"listen");
      checkError((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)),"listen");
      checkError(read(new_socket, &response[0], 1024), "Failed to read data from socket");
      std::cout<<std::endl<<response<<std::endl<<std::endl;

      std::vector<std::string> fileList = getFileList(response);
      for(std::string s : fileList)
        std::cout<<s<<", ";
      std::cout<<std::endl;

      char incomingAdress[16]; // 4x3 digits + 3 dots + \0
      struct in_addr ipAddr = address.sin_addr;
      inet_ntop( AF_INET, &ipAddr, incomingAdress, INET_ADDRSTRLEN );

      addNewPeer(incomingAdress); //adds only if there is no such peer

      std::cout <<"recived some message from: "<<incomingAdress << std::endl;
      std::cout <<"recived contents: "<<response<<std::endl;
      transform(response.begin(), response.end(),response.begin(), ::toupper);
      send(new_socket , response.c_str() , response.length() , 0 );
    }
    return 0;
}
