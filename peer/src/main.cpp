#include "PeerServer.hpp"
#include "CommandsParser.h"
#include <iostream>

int main(int argc, char const *argv[])
{
    PeerServer* peer = PeerServer::instance();
    std::cout << "Peer created!" << std::endl;
    peer->startServer("192.168.20.10", 8080);
    CommandsParser commandsParser(*peer, std::cin, std::cout);
    commandsParser.parseInput();
}
