#include "PeerServer.hpp"
#include "CommandsParser.h"
#include <iostream>

int main(int argc, char const *argv[])
{
    PeerServer peer;
    peer.listenAndServe("192.168.20.10", 8080);
    std::cout << "Program set up finished." << std::endl;
    CommandsParser commandsParser(peer, std::cin, std::cout);
    commandsParser.parseInput();
}
