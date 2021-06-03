#include "PeerClient.hpp"
#include "Config.hpp"
#include <string>
#include <set>
#include <SocketUtils.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <filesystem>
#include <PeerServer.hpp>
#include "CustomUtils.hpp"

namespace fs = std::filesystem;

PeerServer::DownloadResult PeerClient::startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file)
{
    int socket = createListeningClientSocket(file.first.owner, 8080);

    sendMsg(socket,file.first.filename+":"+file.first.owner);                                          //send file name to owner
    long bytesToDownload = std::atol(readMsg(socket).c_str());               //recieve file size in bytes

    if(bytesToDownload == (int)PeerServer::DownloadResult::FILE_REVOKED)
        return PeerServer::DownloadResult::FILE_REVOKED;
    if(bytesToDownload == (int)PeerServer::DownloadResult::FILE_NOT_FOUND)
        return PeerServer::DownloadResult::FILE_NOT_FOUND;

    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path destinationFile = workingDir / (file.first.filename + ":" + file.first.owner);

    long bytesOwned = bytesAlreadyOwned(destinationFile);                    //check if there is some of that file
    sendMsg(socket,std::to_string(bytesOwned));                              //send offset of required data
    std::cout << "Requesting "<<bytesToDownload-bytesOwned<<" bytes of data download"<<std::endl;
    std::cout << "Having "<<bytesOwned<<" bytes already here"<<std::endl;

    std::thread([file, socket, bytesToDownload, bytesOwned, destinationFile] {
        downloadNBytes(socket, bytesToDownload-bytesOwned, destinationFile);
        PeerServer::instance()->addRemoteFile({file.first.filename, file.first.owner});
    }).detach();

    return PeerServer::DownloadResult::DOWNLOAD_OK;
}

void PeerClient::downloadNBytes(int socket, long bytesToDownload, fs::path destinationFile)
{
    progressbar bar(bytesToDownload);
    for(int i=0; i<bytesToDownload;){
        std::ofstream output(destinationFile, std::ios::binary | std::ofstream::app);
        std::string line = readMsg(socket);
        output << line;
        output.close();

        i += (long) line.length();
        bar.update();
    }
}

long PeerClient::bytesAlreadyOwned(std::filesystem::path file)
{
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    if(!in.is_open())
        return 0;
    else
        return in.tellg();
}
