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
    void addFile(std::istream& args);
    void listCommands(const std::set<std::string>& commands);
    void listFiles();
    void listLocalFiles(const std::set<std::string>& filenames);
    bool parseCommand(std::istream& line);
    std::map<std::string, std::function<void(std::istream&)>> commands;
    std::set<std::string> knownCommands;
    PeerServer& peerServer;
    std::istream& in;
    std::ostream& out;
};

#endif //TIN_COMMANDSPARSER_H
