#include "CommandsParser.h"
using namespace std;

CommandsParser::CommandsParser(PeerServer& peerServer_, std::istream& in_, std::ostream& out_) : peerServer(peerServer_), in(in_), out(out_) {
    commands = {
            {"help", [this](istream &restOfLine) { out << "Help :(\n"; }}
    };
}

bool CommandsParser::parseCommand(istream& line) {
    string keyWord;
    line >> keyWord;
    if(keyWord == "exit") {
        out << "Exiting...\n" << endl;
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