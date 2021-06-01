#include "PeerClient.hpp"
#include "Config.hpp"
#include <string>
#include <set>
#include <SocketUtils.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <PeerServer.hpp>
#include "CustomUtils.hpp"

namespace fs = std::filesystem;

PeerServer::DownloadResult PeerClient::startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file)
{
    int socket = createListeningClientSocket(file.first.owner, 8080);

    sendMsg(socket,file.first.filename);                                          //send file name to owner
    long bytesToDownload = std::atol(readMsg(socket).c_str());               //recieve file size in bytes
    if(bytesToDownload<0) return PeerServer::DownloadResult::FILE_NOT_FOUND;

    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path destinationFile = workingDir / (file.first.filename + ":" + file.first.owner);

    long bytesOwned = bytesAlreadyOwned(destinationFile);                  //check if there is some of that file
    sendMsg(socket,std::to_string(bytesOwned));                        //send offset of required data
    std::cout << "Requesting "<<bytesToDownload<<" bytes of data download"<<std::endl;
    std::cout << "Having "<<bytesOwned<<" bytes already here"<<std::endl;

    bytesToDownload -= bytesOwned;
    progressbar bar(bytesToDownload);
    for(int i=0; i<bytesToDownload;){
        std::ofstream output(destinationFile, std::ios::binary | std::ofstream::app);
        std::string line = readMsg(socket);
        output << line;
        output.close();

        i += (long) line.length();
        bar.update();
    }
    std::cout<<std::endl;
    PeerServer::instance()->addFile(destinationFile);
    return PeerServer::DownloadResult::DOWNLOAD_OK;
}

long PeerClient::bytesAlreadyOwned(std::filesystem::path file)
{
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    if(!in.is_open())
        return 0;
    else
        return in.tellg();
}
