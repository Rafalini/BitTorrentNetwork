#include "PeerServer.hpp"

#include "TrackerClient.hpp"
#include <SocketUtils.hpp>

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>

constexpr int HEARTBEAT_INTERVAL = 10;

using namespace std;
namespace fs = std::filesystem;

mutex PeerServer::singletonMutex;
unique_ptr<PeerServer> PeerServer::peerServer;

PeerServer* PeerServer::instance() {
    if(peerServer)
        return peerServer.get();
    singletonMutex.lock();
    if(!peerServer)
        peerServer = unique_ptr<PeerServer>(new PeerServer());
    singletonMutex.unlock();
    return peerServer.get();
}

void PeerServer::lockData() {
    dataMutex.lock();
}

void PeerServer::unlockData() {
    dataMutex.unlock();
}
void PeerServer::lockLocalFiles() {
    fileNamesMutex.lock();
}
void PeerServer::unlockLocalFiles() {
    fileNamesMutex.unlock();
}
void PeerServer::lockDownloadingFiles() {
    downloadingFilesMutex.lock();
}
void PeerServer::unlockDownloadingFiles() {
    downloadingFilesMutex.unlock();
}

void PeerServer::lockCheckDownloadProgress() {
    downloadingFilesMutex.lock();
}
void PeerServer::unlockCheckDownloadProgress() {
    downloadingFilesMutex.unlock();
}

PeerServer::PeerServer() {
    fs::path workingDirectory = fs::current_path();
    workingDirectory /= "bittorrent";
    fs::create_directory(workingDirectory);
    fs::path configFile = workingDirectory / "config";
    if(fs::exists(configFile)) {
        auto data = Config::load(configFile.string());
        localFiles = data["files"];
    } else { //create new config
        Config::Data data;
        data["files"] = {};
        Config::save(configFile.string(),data);
    }
}
void PeerServer::startServer(const std::string &trackerAddr, int port)
{
    sendHeartbeatPeriodically(trackerAddr, port, HEARTBEAT_INTERVAL);
    thread([this,port] { //start listening service
        listenAndServe(port);
    }).detach();
}

//update local information about files
void PeerServer::updateData(const Config::Data& newData) {
    lockData();
    // iterate local files, for each check if their owner didn't delete them
    lockLocalFiles();
    std::set<FileDescriptor> filesToDelete;
    for(auto& fileDescriptor : localFiles) {
        auto& owner = fileDescriptor.owner;
        auto fileOwnerFiles = newData.find(owner);
        if(fileOwnerFiles != newData.end()) {
            auto& files = fileOwnerFiles->second;
            if(std::find(files.begin(), files.end(), fileDescriptor) == files.end())
                filesToDelete.insert(fileDescriptor);
        }
    }
    for(auto& fileToDelete : filesToDelete)
        deleteFile(fileToDelete);
    unlockLocalFiles();
    if(data != newData) {
        data = newData;
    }
    unlockData();
}


bool PeerServer::stopDownloadingFile(const string& filename, const string& owner) {
    lockDownloadingFiles();
    auto downloadingFile = std::find_if(downloadingFiles.begin(),
                                        downloadingFiles.end(),
                                        [filename, owner](const auto& fileDescriptor){
                                            return filename == std::get<0>(fileDescriptor).filename &&
                                                   (owner.empty() || std::get<0>(fileDescriptor).owner == owner);
                                        } );
    if(downloadingFile == downloadingFiles.end()) {
        unlockDownloadingFiles();
        return false;
    }
    downloadingFiles.erase(downloadingFile);
    unlockDownloadingFiles();
    return true;
}

bool PeerServer::checkDownloadProgress(const std::string& fileName, const std::string& owner) {
    lockCheckDownloadProgress();
    auto downloadingFile = std::find_if(downloadingFiles.begin(),
                                        downloadingFiles.end(),
                                        [fileName, owner](const auto& fileDescriptor){
                                            return fileName == std::get<0>(fileDescriptor).filename &&
                                                  (owner.empty() || std::get<0>(fileDescriptor).owner == owner);
                                        } );
    if(downloadingFile == downloadingFiles.end()) {
        unlockCheckDownloadProgress();
        return false;
    }

    std::cout << "File: " << (std::get<0>(*downloadingFile)).filename <<
                ", owner: " << (std::get<0>(*downloadingFile)).owner <<
                ": downloaded " << std::get<1>(*downloadingFile) <<
                " bytes of " << std::get<2>(*downloadingFile) << " bytes.\n";
    unlockCheckDownloadProgress();
    return true;
}

PeerServer::DownloadResult PeerServer::startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file)
{
    int socket = createListeningClientSocket(file.first.owner, 8080);

    sendMsg(socket,file.first.filename+":"+file.first.owner);                                          //send file name to owner
    long bytesToDownload = std::atol(readMsg(socket).c_str());               //recieve file size in bytes

    if(bytesToDownload == (int)PeerServer::DownloadResult::FILE_REVOKED)
        return DownloadResult::FILE_REVOKED;\

    // FIXME the same error message if file not found locally or remotely
    if(bytesToDownload == (int)PeerServer::DownloadResult::FILE_NOT_FOUND)
        return DownloadResult::FILE_NOT_FOUND;

    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path destinationFile = workingDir / (file.first.filename + ":" + file.first.owner);

    long bytesOwned = bytesAlreadyOwned(destinationFile);                    //check if there is some of that file
    sendMsg(socket,std::to_string(bytesOwned));                              //send offset of required data
    std::cout << "Requesting "<<bytesToDownload-bytesOwned<<" bytes of data download"<<std::endl;
    std::cout << "Having "<< bytesOwned <<" bytes already here"<<std::endl;

    lockDownloadingFiles();
    downloadingFiles.push_back({file.first, bytesOwned, bytesToDownload});
    unlockDownloadingFiles();

    std::thread([file, socket, bytesToDownload, bytesOwned, destinationFile, this] {
        bool result = downloadNBytes(socket, bytesToDownload-bytesOwned, destinationFile, file.first);
        if(result) {
            PeerServer::instance()->addRemoteFile({file.first.filename, file.first.owner});
            std::cout << "Downloading finished for " << destinationFile.filename() << std::endl;
        }
        else
            fs::remove(destinationFile);
    }).detach();

    return DownloadResult::DOWNLOAD_OK;
}

bool PeerServer::downloadNBytes(int socket, long bytesToDownload, fs::path destinationFile, const FileDescriptor& file)
{
    for(long i=0; i<bytesToDownload;){
        sleep(4);
        std::string line = readMsg(socket);
        std::ofstream output(destinationFile, std::ios::binary | std::ofstream::app);
        output << line;
        output.close();

        i += (long) line.length();

        lockDownloadingFiles();
        auto downloadingFile = std::find_if(downloadingFiles.begin(),
                                           downloadingFiles.end(),
                                           [file](const auto& fileDescriptor){
                                               return file.filename == std::get<0>(fileDescriptor).filename &&
                                                      (file.owner.empty() || std::get<0>(fileDescriptor).owner == file.owner);
                                           } );
        if(downloadingFile != downloadingFiles.end()){
            std::get<1>(*downloadingFile) = i;
        }
        else{
            unlockDownloadingFiles();
            return false;
        }
        unlockDownloadingFiles();
    }
    lockDownloadingFiles();
    downloadingFiles.erase(std::remove(
            downloadingFiles.begin(), downloadingFiles.end(),
            std::tuple<FileDescriptor, long, long>(file, bytesToDownload,bytesToDownload)), downloadingFiles.end());
    unlockDownloadingFiles();
    return true;
}

long PeerServer::bytesAlreadyOwned(std::filesystem::path file)
{
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    if(!in.is_open())
        return 0;
    else
        return in.tellg();
}

PeerServer::DownloadResult PeerServer::downloadFile(const string& fileName, const string& owner) {
    lockData();
    auto transformedData = transformData();
    unlockData();
    std::map<FileDescriptor, std::set<std::string>> foundFiles;
    for_each(transformedData.begin(), transformedData.end(), [&foundFiles, &fileName](const auto& file) {
        if(file.first.filename == fileName)
            foundFiles.insert(file);
    });
    if(foundFiles.size() == 0) {
        return DownloadResult::FILE_NOT_FOUND;
    }
    if(foundFiles.size() == 1) {
        const FileDescriptor fileToDownload(fileName, owner);
        auto downloadedFile = std::find_if(localFiles.begin(), localFiles.end(), [fileToDownload](const auto& fileDescriptor){
            return fileToDownload.filename == fileDescriptor.filename &&
                   (fileToDownload.owner.empty() || fileDescriptor.owner == fileToDownload.owner);
        } );
        if(downloadedFile != localFiles.end())
            return DownloadResult::FILE_ALREADY_PRESENT;
        auto downloadingFile = std::find_if(downloadingFiles.begin(), downloadingFiles.end(), [fileToDownload](const auto& fileDescriptor){
            return fileToDownload.filename == std::get<0>(fileDescriptor).filename &&
                   (fileToDownload.owner.empty() || std::get<0>(fileDescriptor).owner == fileToDownload.owner);
        } );
        if(downloadingFile != downloadingFiles.end())
            return DownloadResult::FILE_ALREADY_BEING_DOWNLOADED;
        return startDownloadingFile(*foundFiles.begin());
    }
    for(const auto& file : foundFiles) {
        if(file.first.owner == owner) {
            const FileDescriptor fileToDownload = FileDescriptor(fileName, owner);
            auto downloadedFile = std::find_if(localFiles.begin(), localFiles.end(), [fileToDownload](const auto& fileDescriptor){
                return fileToDownload.filename == fileDescriptor.filename &&
                       fileDescriptor.owner == fileToDownload.owner;
            } );
            if(downloadedFile != localFiles.end())
                return DownloadResult::FILE_ALREADY_PRESENT;
            auto downloadingFile = std::find_if(downloadingFiles.begin(), downloadingFiles.end(), [fileToDownload](const auto& fileDescriptor){
                return fileToDownload.filename == std::get<0>(fileDescriptor).filename &&
                       std::get<0>(fileDescriptor).owner == fileToDownload.owner;
            } );
            if(downloadingFile != downloadingFiles.end())
                return DownloadResult::FILE_ALREADY_BEING_DOWNLOADED;
            return startDownloadingFile(file);
        }
    }
    return DownloadResult::FILE_NOT_FOUND;
}

[[noreturn]] void PeerServer::listenAndServe(int port){ //run upload threads on upcoming download requests
    auto sock = createListeningServerSocket(port);
    while (true) {
        struct sockaddr_in client = {0};
        unsigned int size = sizeof(client);
        int downloadSocket = accept(sock, (struct sockaddr *) &client, &size);
        if (downloadSocket == -1)
            std::cerr << "couldn't accept connection from peer";

        std::thread([this, downloadSocket, client] {
            std::string clientIP = inet_ntoa(client.sin_addr);
            handleDownloadRequest(downloadSocket);
            close(downloadSocket);
        }).detach();
    }
}

FileDescriptor PeerServer::getDescriptor(std::string fileId){
    std::stringstream test(fileId);
    std::string segment;
    std::vector<std::string> parts;

    while(std::getline(test, segment, ':'))
        parts.push_back(segment);

    return FileDescriptor(parts[0],parts[1]);
}

void PeerServer::handleDownloadRequest(int socket) //upload file
{
    std::string fileId = readMsg(socket);
    FileDescriptor fileToDownload =  getDescriptor(fileId);

    std::cout<<std::endl<<"Received download request for: "<<fileToDownload.filename<<std::endl;
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path destinationFile = workingDir / fileId;

    if(isFileRevoked()) {
        sendMsg(socket,std::to_string((int)PeerServer::DownloadResult::FILE_REVOKED));
        return;
    }

    long bytesToUpload = fileSize(destinationFile);     //get file size
    sendMsg(socket, std::to_string(bytesToUpload));    //send file size
    if(bytesToUpload<0) return;                             //no such file found, client will know that too

    long offset = 0;
    offset = std::atol(readMsg(socket).c_str());      //if client has some part of that file, set offset
    std::cout << "client has already: "<<offset<<std::endl;

    uploadNBytes(socket, bytesToUpload-offset, offset, destinationFile);
    std::cout << "Uploaded "<< bytesToUpload <<" bytes";
    std::cout << std::endl<<":~$ ";
}

void PeerServer::uploadNBytes(int socket, long bytesToUpload, long offset, fs::path destinationFile)
{
    for(long i=offset; i < bytesToUpload;) {
        ifstream input(destinationFile, ios::binary);
        input.seekg(offset + i, ios_base::beg);     //move to last unread chunk

        long nextChunkSize = chunkSize;
        if(nextChunkSize > bytesToUpload)
            nextChunkSize = bytesToUpload;

        std::string line(nextChunkSize,{0});                 //prepare buffer
        input.read(&line[0], line.length());
        line.resize(input.gcount());                         //if less bytes were red than chunkSize, cut cline to fit

        sendMsg(socket, line);
        input.close();
        usleep(1000*300);                           //microseconds -- delay
        i += (long) input.gcount();
    }
}

long PeerServer::fileSize(std::filesystem::path file)
{
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    if(!in.is_open())
        return -1;
    else
        return in.tellg();
}

std::map<FileDescriptor, std::set<std::string>> PeerServer::transformData() {
    std::map<FileDescriptor, std::set<std::string>> dataTransformed;
    for(auto&[owner, filenames] : data ) {
        for (auto &file : filenames) {
            auto &fileOwners = dataTransformed[file];
            fileOwners.insert(owner);
        }
    }
    return dataTransformed;
}

void PeerServer::sendHeartbeatPeriodically(const std::string& trackerAddr, int port, unsigned int interval) {
    auto [newData, ipAddr] = sendHeartbeat(trackerAddr, port);
    myAddr = ipAddr;
    thread([this, trackerAddr, port, interval] {
        //a little overhead at start, but guaranties that ipaddr is not changes during run
        auto [newData, ipAddr] = sendHeartbeat(trackerAddr, port);
        if(localName != ipAddr)
            localName = ipAddr;
        updateData(newData);
        while (running) {
            auto x = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            auto [newData, _] = sendHeartbeat(trackerAddr, port);
            updateData(newData);
            this_thread::sleep_until(x);
        }
    }).detach();
}

DataAndIp PeerServer::sendHeartbeat(const std::string& trackerAddr, int port) {
    auto received = TrackerClient::sendData(trackerAddr, port, localFiles);
    return received;
}

std::string PeerServer::getMyAddr() {return myAddr;}

bool PeerServer::addRemoteFile(const FileDescriptor& file) {
    lockLocalFiles();
    if(localFiles.find(file) != localFiles.end()) {
        unlockLocalFiles();
        return false;
    }
    localFiles.insert(file);
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::create_directory(workingDir);
    fs::path destinationFile = workingDir / (file.filename + ":" + file.owner);
    if(!fs::exists(destinationFile)) {
        fs::path configFile = workingDir / "config";
        Config::Data data;
        data["files"] = localFiles;
        Config::encodeConfig(data);
        Config::save(configFile, data);
    }
    unlockLocalFiles();
    return true;
}

bool PeerServer::addFile(const fs::path& fromPath) {
    string newFileName = fromPath.filename();
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::create_directory(workingDir);
    lockLocalFiles();
    if(localFiles.find({newFileName, localName}) != localFiles.end()) {
        unlockLocalFiles();
        return false;
    }
    localFiles.insert({newFileName, localName});
    fs::path destinationFile = workingDir / (newFileName + ":" + localName);
    if(!fs::exists(destinationFile)) {
        fs::copy(fromPath, destinationFile);
        fs::path configFile = workingDir / "config";
        Config::Data data;
        data["files"] = localFiles;
        Config::encodeConfig(data);
        Config::save(configFile, data);
    }
    unlockLocalFiles();
    return true;
}

void PeerServer::deleteFile(const FileDescriptor& fileDescriptor) {
    string fileNameToDelete = fileDescriptor.filename + ":" + fileDescriptor.owner;
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path fileToDelete = workingDir / fileNameToDelete;
    if(localFiles.find(fileDescriptor) == localFiles.end()) {
        return;
    }
    localFiles.erase(fileDescriptor);
    if(fs::exists(fileToDelete)) {
        fs::remove(fileToDelete);
    }
    fs::path configFile = workingDir / "config";
    Config::Data data;
    data["files"] = localFiles;
    Config::encodeConfig(data);
    Config::save(configFile, data);
}

bool PeerServer::isFileRevoked()
{
    return false;
}
