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
            {"add-file", [this](istream &restOfLine) { addFile(restOfLine); }},
            {"download-file", [this](istream &restOfLine) { downloadFile(restOfLine); }},
            {"delete-file", [this](istream &restOfLine){ deleteFile(restOfLine); }},
            {"stop-downloading-file", [this](istream &restOfLine){ stopDownloadingFile(restOfLine); }},
            {"check-download-progress", [this](istream &restOfLine){ checkDownloadProgress(restOfLine); }},
            {"copy-file", [this](istream &restOfLine){ copyFile(restOfLine); }}
    };
    knownCommands = {"exit", "help", "list-files", "list-local-files", "add-file file_path", "download-file filename [original_owner]",
                     "delete-file filename [original_owner]", "stop-downloading-file filename [original_owner]",
                     "check-download-progress filename [original_owner]", "copy-file path_to_save_as filename [original_owner]"};
}

void CommandsParser::addFile(istream& args) {
    fs::path pathToFile;
    args >> pathToFile;
    if(pathToFile.empty()) {
        out << "Error: Path to file not provided.\n";
        return;
    }
    if(!fs::exists(pathToFile)) {
        out << "Error: File doesn't exist\n";
        return;
    }
    if(peerServer.addFile(pathToFile))
        out << pathToFile.filename() << " saved\n";
    else
        out << "File was already added\n";
}

void CommandsParser::downloadFile(istream& args) {
    string fileName, owner;
    args >> fileName >> owner;
    auto result = peerServer.downloadFile(fileName, owner);
    switch(result) {
        case PeerServer::DownloadResult::DOWNLOAD_OK:
            out << "Downloading of " << fileName << " has begun.\n";
            return;
        case PeerServer::DownloadResult::FILE_ALREADY_PRESENT:
            out << "File is already available locally\n";
            return;
        case PeerServer::DownloadResult::FILE_NOT_FOUND:
            out << "File not found\n";
            return;
        case PeerServer::DownloadResult::DOWNLOAD_ABORTED:
            out << "Download aborted\n";
            return;
        case PeerServer::DownloadResult::FILE_REVOKED:
            out << "Download aborted, file revoked\n";
            return;
        case PeerServer::DownloadResult::FILE_ALREADY_BEING_DOWNLOADED:
            out << "This file is already being downloaded\n";
            return;
        case PeerServer::DownloadResult::FILE_NOT_AVAILABLE_TO_DOWNLOAD:
            out << "None of the other peers owning this file is available at the moment. Please try again later\n";
            return;
    }
}

void CommandsParser::copyFile(istream& args) {
    fs::path pathToFile;
    args >> pathToFile;
    std::cout << "path:" << pathToFile.string() << ";\n";

    string filename;
    args >> filename;
    if(filename.empty()) {
        out << "Error: Path to file to delete not provided.\n";
        return;
    }
    string owner;
    args >> owner;
    peerServer.lockLocalFiles();
    auto fileDescriptor = std::find_if(peerServer.getLocalFiles().begin(),
                                       peerServer.getLocalFiles().end(),
                                       [&filename, &owner](const FileDescriptor& fileDescriptor){
                                           return fileDescriptor.filename == filename &&
                                                  (owner.empty() || fileDescriptor.owner == owner);
                                       } );
    peerServer.unlockLocalFiles();
    if(fileDescriptor == peerServer.getLocalFiles().end()) {
        out << "Error: File to copy doesn't exist\n";
        return;
    }
    fs::copy("bittorrent/" + (*fileDescriptor).filename + ":" + (*fileDescriptor).owner, pathToFile);

    out << (*fileDescriptor).filename << "(" << (*fileDescriptor).owner << ") copied to " << pathToFile.string() << "\n";
}


void CommandsParser::deleteFile(istream& args) {
    string filename;
    args >> filename;
    if(filename.empty()) {
        out << "Error: Path to file to delete not provided.\n";
        return;
    }
    string owner;
    args >> owner;
    peerServer.lockLocalFiles();
    auto fileDescriptor = std::find_if(peerServer.getLocalFiles().begin(),
                                        peerServer.getLocalFiles().end(),
                                        [&filename, &owner](const FileDescriptor& fileDescriptor){
                                            return fileDescriptor.filename == filename &&
                                            (owner.empty() || fileDescriptor.owner == owner);
                                        } );
    peerServer.unlockLocalFiles();
    if(fileDescriptor == peerServer.getLocalFiles().end()) {
        out << "Error: File to delete doesn't exist\n";
        return;
    }
    peerServer.lockLocalFiles();
    peerServer.deleteFile(*fileDescriptor);
    peerServer.unlockLocalFiles();
    out << filename << " deleted\n";
}

void CommandsParser::stopDownloadingFile(istream& args) {
    string filename;
    args >> filename;
    string owner;
    args >> owner;
    if(filename.empty()) {
        out << "Error: Path to file not provided.\n";
        return;
    }
    if(peerServer.stopDownloadingFile(filename, owner)) {
        out << "Stopped downloading " << filename << "\n";
    } else {
        out << "Nothing to stop\n";
    }
}

void CommandsParser::checkDownloadProgress(istream& args) {
    string filename;
    args >> filename;
    string owner;
    args >> owner;
    if(filename.empty()) {
        out << "Error: Path to file not provided.\n";
        return;
    }
    if (!peerServer.checkDownloadProgress(filename, owner))
        out << "Error: There is no such file in the process of downloading.\n";

}

void CommandsParser::listCommands(const vector<string>& commands) {
    out << "Known commands:\n";
    for(auto& command: commands )
        out << command << "\n";
    out << "owner is ignored if there is only one instance of file with the given name\n";
}

void CommandsParser::listFiles() {
    out << "Files available to download:\n";
    peerServer.lockData();
//    auto data = peerServer.getData();
    auto dataTransformed = peerServer.transformData();
    peerServer.unlockData();
    for(auto& [file, owners] : dataTransformed )
        out << file.filename << "(" << file.owner  << "): from " << owners.size() << " sources\n";
}

void CommandsParser::listLocalFiles(const std::set<FileDescriptor>& files) {
    out << "Files available locally:\n";
    for(auto& file : files )
        out << file.filename << "(" << file.owner << ")"<< "\n";
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
    std::cout<<PeerServer::instance()->getMyAddr()<<":~$ ";
    string line;
    getline(in, line);
    stringstream lineStringStream(line);
    while(parseCommand(lineStringStream)) {
        std::cout<<PeerServer::instance()->getMyAddr()<<":~$ ";
        getline(in, line);
        lineStringStream = stringstream(line);
    }
}