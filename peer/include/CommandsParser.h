#ifndef TIN_COMMANDSPARSER_H
#define TIN_COMMANDSPARSER_H
#include "PeerServer.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <functional>

class CommandsParser {
public:
    CommandsParser(PeerServer& peerServer, std::istream& in, std::ostream& out);
    void parseInput();
private:
    void downloadFile(std::istream& args);
    void addFile(std::istream& args);
    void listCommands(const std::vector<std::string>& commands);
    void listFiles();
    void listLocalFiles(const std::set<FileDescriptor>& files);
    bool parseCommand(std::istream& line);
    std::map<std::string, std::function<void(std::istream&)>> commands;
    std::vector<std::string> knownCommands;
    PeerServer& peerServer;
    std::istream& in;
    std::ostream& out;
};

#endif //TIN_COMMANDSPARSER_H
