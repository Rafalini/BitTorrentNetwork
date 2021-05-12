#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>

#define PORT 8080
#define IPPROTOCOL 0

using namespace std;

void addNewPeer(string adress);
bool ifcontains(string adress);

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    ofstream configFile;


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

    for(;;) //forever together love etc
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

      string response(1024, 0);
      if (read(new_socket, &response[0], 1024)<0) {
          cerr << "Failed to read data from socket.\n";
      }
      char incomingAdress[16]; // 4x3 digits + 3 dots + \0
      struct in_addr ipAddr = address.sin_addr;
      char str[INET_ADDRSTRLEN];
      inet_ntop( AF_INET, &ipAddr, incomingAdress, INET_ADDRSTRLEN );

      addNewPeer(incomingAdress); //adds only if there is no such peer

      cout <<"recived some message from: "<<incomingAdress << endl;
      cout <<"recived contents: "<<response<<endl;
      transform(response.begin(), response.end(),response.begin(), ::toupper);
      send(new_socket , response.c_str() , response.length() , 0 );
    }
    return 0;
}

void addNewPeer(string adress)
{
  if(!ifcontains(adress))
  {
    ofstream configFile;
    configFile.open("peerList.cfg", ios::out | ios::app | ios::binary);
    configFile << adress << endl;
    configFile.close();
  }
}

bool ifcontains(string adress)
{
  ifstream configFile ("peerList.cfg");
  string content;

  while(getline(configFile,content))
    if(content == adress)
      return true;
  configFile.close();
  return false;
}
