#include "CommandsParser.h"
#include "Config.hpp"
using namespace std;

CommandsParser::CommandsParser(PeerServer& peerServer_, std::istream& in_, std::ostream& out_) : peerServer(peerServer_), in(in_), out(out_) {
    commands = {
            {"help", [this](istream &restOfLine) { listCommands(out, knownCommands); }},
            {"list-files", [this](istream &restOfLine) { listFiles(out, peerServer.getData()); }},
            {"list-local-files", [this](istream &restOfLine) { listLocalFiles(out, peerServer.getFileNames()); }}
    };
    knownCommands = {"help", "list-files", "list-local-files"};
}

ostream& CommandsParser::listCommands(ostream& outStream, const set<string>& commands) {
    outStream << "Known commands:\n";
    for(auto& command: commands )
        outStream << command << "\n";
    return outStream;
}

ostream& CommandsParser::listFiles(ostream& outStream, const Data& data) {
    outStream << "Files available locally:\n";
    for(auto& [filename, owners] : data )
        outStream << filename << ": from " << owners.size() << "sources\n";
    return outStream;
}

ostream& CommandsParser::listLocalFiles(ostream& outStream, const std::set<std::string>& filenames) {
    outStream << "Files available locally:\n";
    for(auto& filename : filenames )
        outStream << filename << "sources\n";
    return outStream;
}

bool CommandsParser::parseCommand(istream& line) {
    string keyWord;
    line >> keyWord;
    if(keyWord == "exit") {
        out << "Exiting...\n" << endl;
        peerServer.stop();
        return false;
    }
    auto fun = commands.find(keyWord);
    if(fun != commands.end()) {
        fun->second(line);
    } else {
        out << "Unrecognized command. Try \"help\" for the list of possible commands\n";
    }
    return true;
}

void CommandsParser::parseInput() {
    out << "Please type in \"help\" to get the list of commands" << std::endl;
    string line;
    getline(in, line);
    stringstream lineStringStream(line);
    while(parseCommand(lineStringStream)) {
        getline(in, line);
        lineStringStream = stringstream(line);
    }
}