#include "PeerServer.hpp"

#include "TrackerClient.hpp"
#include <SocketUtils.hpp>

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

void PeerServer::listenAndServe(const std::string &trackerAddr, int port) {
    auto sock = createListeningServerSocket(port);
    sendHeartbeatPeriodically(trackerAddr, port, HEARTBEAT_INTERVAL);
    thread([this, sock] {
        while (running) {
            struct sockaddr_in client = {0};
            unsigned int size = sizeof(client);
            int msgSocket = accept(sock, (struct sockaddr *) &client, &size);
            if (msgSocket == -1)
                cerr << "couldn't accept connection from peer";
            close(msgSocket);
        }
    }).detach();
}

void PeerServer::updateData(const Config::Data& newData) {
    lockData();
    if(data != newData) {
        data = newData;
    }
    unlockData();
}

PeerServer::DownloadResult PeerServer::downloadFile(const string& fileName, const string& owner) {
    lockData();
    auto transformedData = transformData(data);
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
        startDownloadingFile(*foundFiles.begin());
        return DownloadResult::DOWNLOAD_OK;
    }
    for(const auto& file : foundFiles) {
        if(file.first.owner == owner) {
            startDownloadingFile(file);
            return DownloadResult::DOWNLOAD_OK;
        }
    }
    return DownloadResult::FILE_NOT_FOUND;
}

void PeerServer::startDownloadingFile(const std::pair<FileDescriptor, set<string>>& file) {
    std::cout << "DOWNLOADING WOULD BE STARTED NOW\n";
}

std::map<FileDescriptor, std::set<std::string>> PeerServer::transformData(const Config::Data &) {
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
    thread([this, trackerAddr, port, interval] {
        //a little overhead at start, but guaranties that ipaddr is not changes during run
        auto [newData, ipAddr] = sendHeartbeat(trackerAddr, port);
        if(localName != ipAddr)
            localName = ipAddr;
        updateData(newData);
        while (running) {
            auto x = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            std::cout << "HeartBeat, now"<<std::endl;
            auto [newData, _] = sendHeartbeat(trackerAddr, port);
            updateData(newData);
            this_thread::sleep_until(x);
        }
    }).detach();
}

DataAndIp PeerServer::sendHeartbeat(const std::string& trackerAddr, int port) {
    auto received = TrackerClient::sendData(trackerAddr, port, localFiles);
    std::cout << "Heartbeat update, new data received\n";
    return received;
}

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
        fs::create_directory(workingDir);
        fs::path configFile = workingDir / "config";
        Config::Data data;
        data["files"] = localFiles;
        Config::encodeConfig(data);
        Config::save(configFile, data);
    }
    unlockLocalFiles();
    return true;
}
