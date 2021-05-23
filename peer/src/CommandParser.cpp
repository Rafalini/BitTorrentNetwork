#include "CommandsParser.h"
#include "Config.hpp"
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

CommandsParser::CommandsParser(PeerServer& peerServer_, std::istream& in_, std::ostream& out_) : peerServer(peerServer_), in(in_), out(out_) {
    commands = {
            {"help", [this](istream &restOfLine) { listCommands(knownCommands); }},
            {"list-files", [this](istream &restOfLine) { listFiles(); }},
            {"list-local-files", [this](istream &restOfLine) { listLocalFiles(peerServer.getLocalFiles()); }},
            {"add-file", [this](istream &restOfLine) { addFile(restOfLine); }}
    };
    knownCommands = {"exit", "help", "list-files", "list-local-files", "add-file file_path", "download-file filename"};
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
    out << "Files available to download:\n";
    peerServer.lockData();
    auto data = peerServer.getData();
    std::map<FileDescriptor, std::set<std::string>> dataTransformed;
    for(auto& [owner, filenames] : data ) {
        for(auto& file : filenames) {
            auto& fileOwners = dataTransformed[file];
            fileOwners.insert(owner);
        }
    }
    peerServer.unlockData();
    for(auto& [file, owners] : dataTransformed )
        out << file.filename  << /*" recognized by ownership of "*/"(" << file.owner  << "): from " << owners.size() << " sources\n";
}

void CommandsParser::listLocalFiles(const std::set<FileDescriptor>& files) {
    out << "Files available locally:\n";
    for(auto& file : files )
        out << file.filename << "\n";
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