#ifndef TIN_COMMANDSPARSER_H
#define TIN_COMMANDSPARSER_H
#include "PeerServer.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <functional>

class CommandsParser {
public:
    CommandsParser(PeerServer& peerServer, std::istream& in, std::ostream& out);
    void parseInput();
private:
    std::ostream& listCommands(std::ostream& outStream, const std::set<std::string>& commands);
    std::ostream& listFiles(std::ostream& outStream, const Data& data);
    std::ostream& listLocalFiles(std::ostream& outStream, const std::set<std::string>& filenames);
    bool parseCommand(std::istream& line);
    std::map<std::string, std::function<void(std::istream&)>> commands;
    std::set<std::string> knownCommands;
    PeerServer& peerServer;
    std::istream& in;
    std::ostream& out;
};

#endif //TIN_COMMANDSPARSER_H
