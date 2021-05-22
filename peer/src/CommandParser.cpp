#include "CommandsParser.h"
#include "Config.hpp"
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

CommandsParser::CommandsParser(PeerServer& peerServer_, std::istream& in_, std::ostream& out_) : peerServer(peerServer_), in(in_), out(out_) {
    commands = {
            {"help", [this](istream &restOfLine) { listCommands(knownCommands); }},
            {"list-files", [this](istream &restOfLine) { listFiles(); }},
            {"list-local-files", [this](istream &restOfLine) { listLocalFiles(peerServer.getFileNames()); }},
            {"add-file", [this](istream &restOfLine) { addFile(restOfLine); }}
    };
    knownCommands = {"help", "list-files", "list-local-files", "add-file file_path"};
}

void CommandsParser::addFile(istream& args) {
    string pathToFile;
    args >> pathToFile;
    if(pathToFile.empty()) {
        out << "Error: Path to file not provided.\n";
        return;
    }
    if(!fs::exists(pathToFile)) {
        out << "Error: File doesn't exist\n";
        return;
    }
    fs::path fromPath(pathToFile);
    if(peerServer.addFile(fromPath))
        out << fromPath.filename() << " saved\n";
    else
        out << "File was already added\n";
}

void CommandsParser::listCommands(const set<string>& commands) {
    out << "Known commands:\n";
    for(auto& command: commands )
        out << command << "\n";
}

void CommandsParser::listFiles() {
    out << "Files available locally:\n";
    peerServer.lockData();
    auto data = peerServer.getData();
    std::map<std::string, std::set<std::string>> dataTransformed;
    for(auto& [owner, filenames] : data ) {
        for(auto& filename : filenames) {
            auto& fileOwners = dataTransformed[filename];
            fileOwners.insert(owner);
        }
    }
    peerServer.unlockData();
    for(auto& [filename, owners] : dataTransformed )
        out << filename << ": from " << owners.size() << " sources\n";
}

void CommandsParser::listLocalFiles(const std::set<std::string>& filenames) {
    out << "Files available locally:\n";
    for(auto& filename : filenames )
        out << filename << "\n";
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