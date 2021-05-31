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

    sendMsg(socket,file.first.filename);                                  //send file name to owner
    long bytesToDownload = std::atol(readMsg(socket).c_str());      //get file size in bytes

    std::cout << "Requesting "<<bytesToDownload<<" bytes of data download"<<std::endl;

    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path destinationFile = workingDir / (file.first.filename + ":" + file.first.owner);

    progressbar bar(bytesToDownload);
    for(int i=0; i<bytesToDownload;){
        std::ofstream output(destinationFile, std::ios::binary | std::ofstream::app);

        std::string line = readMsg(socket);
        output.close();

        i += (long) line.length();
        bar.update();

//        if(false)
//            return PeerServer::DownloadResult::DOWNLOAD_ABORTED;
    }
    std::cout<<std::endl;
    PeerServer::instance()->addFile(destinationFile);
    return PeerServer::DownloadResult::DOWNLOAD_OK;
}
