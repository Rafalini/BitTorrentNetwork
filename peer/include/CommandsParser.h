#ifndef TIN_COMMANDSPARSER_H
#define TIN_COMMANDSPARSER_H
#include "PeerServer.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <functional>

class CommandsParser {
public:
    CommandsParser(PeerServer& peerServer, std::istream& in, std::ostream& out);
    void parseInput();
private:
    bool parseCommand(std::istream& line);
    std::map<std::string, std::function<void(std::istream&)>> commands;
    PeerServer& peerServer;
    std::istream& in;
    std::ostream& out;
};

#endif //TIN_COMMANDSPARSER_H
