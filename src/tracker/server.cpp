#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <algorithm>
#include <string>
#include <iostream>
#define PORT 8080
#define IPPROTOCOL 0

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

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
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
      if (listen(server_fd, 3) < 0)
      {
          perror("listen");
          exit(EXIT_FAILURE);
      }
      if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
      {
          perror("accept");
          exit(EXIT_FAILURE);
      }

      std::string response(1024, 0);
      if (read(new_socket, &response[0], 1024)<0) {
          std::cerr << "Failed to read data from socket.\n";
      }
      char incomingAdress[10];
      struct in_addr ipAddr = address.sin_addr;
      char str[INET_ADDRSTRLEN];
      inet_ntop( AF_INET, &ipAddr, incomingAdress, INET_ADDRSTRLEN );

      std::cout <<"recived some message from: "<<incomingAdress << std::endl;
      std::cout <<"recived contents: "<<response<<std::endl;
      std::transform(response.begin(), response.end(),response.begin(), ::toupper);
      send(new_socket , response.c_str() , response.length() , 0 );
    }
    return 0;
}
