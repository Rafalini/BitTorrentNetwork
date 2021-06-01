#include "PeerServer.hpp"
#include "PeerClient.hpp"

#include "TrackerClient.hpp"
#include <SocketUtils.hpp>

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
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
    if(data != newData) {
        data = newData;
    }
    unlockData();
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
        PeerClient::startDownloadingFile(*foundFiles.begin());
        return DownloadResult::DOWNLOAD_OK;
    }
    for(const auto& file : foundFiles) {
        if(file.first.owner == owner) {
            PeerClient::startDownloadingFile(file);
            return DownloadResult::DOWNLOAD_OK;
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

void PeerServer::handleDownloadRequest(int msgSocket) //upload file
{
    std::string fileName = readMsg(msgSocket);

    std::cout<<std::endl<<"Received download request for: "<<fileName<<std::endl;
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::path destinationFile = workingDir / (fileName + ":" + localName);

    long bytesToUpload = fileSize(destinationFile);     //get file size
    sendMsg(msgSocket, std::to_string(bytesToUpload)); //send file size
    if(bytesToUpload<0) return;                             //no such file found, client will know that too

    long offset = 0;
    offset = std::atol(readMsg(msgSocket).c_str());      //if client has some part of that file, set offset
    std::cout << "client has already: "<<offset<<std::endl;

    for(int i=0; bytesToUpload>0; ++i) {
        ifstream input(destinationFile, ios::binary);
        input.seekg(offset+i*chunkSize, ios_base::beg);     //move to last unread chunk
        std::string line(chunkSize,{0});                 //prepare buffer
        input.read(&line[0], line.length());
        line.resize(input.gcount());                         //if less bytes were red than chunkSize, cut cline to fit

        sendMsg(msgSocket, line);
        input.close();
        usleep(1000*300);                           //microseconds -- delay
        bytesToUpload -= (long) input.gcount();
    }
    std::cout << "Uploaded "<< fileSize(destinationFile) <<" bytes";
    std::cout << std::endl<<":~$ ";
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

bool PeerServer::addFile(const fs::path& fromPath) {
    string newFileName = fromPath.filename();
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::create_directory(workingDir);
    lockLocalFiles();
    if(localFiles.find({newFileName, localName}) != localFiles.end())
        return false;
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
