#include "PeerServer.hpp"

#include "TrackerClient.hpp"
#include <SocketUtils.hpp>

#include <iostream>
#include <chrono>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
constexpr int HEARTBEAT_INTERVAL = 30;

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
void PeerServer::lockFileNames() {
    fileNamesMutex.lock();
}

void PeerServer::unlockFileNames() {
    fileNamesMutex.unlock();
}


PeerServer::PeerServer() {
    fs::path workingDirectory = fs::current_path();
    workingDirectory /= "bittorrent";
    fs::create_directory(workingDirectory);
    for (const auto & file : fs::directory_iterator(workingDirectory)) {
        if(!file.is_directory()) {
            fileNames.insert(file.path().filename().string());
        }
    }
}

void PeerServer::listenAndServe(const std::string &trackerAddr, int port) {
    sendHeartbeatPeriodically(trackerAddr, port, HEARTBEAT_INTERVAL);
    thread([this, trackerAddr, port] {
        int sock = createListeningServerSocket(port);
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

void PeerServer::sendHeartbeatPeriodically(const std::string& trackerAddr, int port, unsigned int interval) {
    thread([this, trackerAddr, port, interval] {
        while (running) {
            auto x = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            Data newData = sendHeartbeat(trackerAddr, port);
            lockData();
            if(data != newData) {
                data = newData;
            }
            unlockData();
            this_thread::sleep_until(x);
        }
    }).detach();
}

std::map<std::string, std::set<std::string>> PeerServer::sendHeartbeat(const std::string& trackerAddr, int port) {
    auto received = TrackerClient::sendData(trackerAddr, port, fileNames);
//  Uncomment to check received data
    for (const auto &peer : received) {
        std::cout << peer.first << ": ";
        for (const auto &file : peer.second) {
            std::cout << file << ", ";
        }
        std::cout << std::endl;
    }
    return received;
}

bool PeerServer::addFile(const fs::path& fromPath) {
    fs::path newFileName = fromPath.filename();
    fs::path destinationPath = fs::current_path();
    destinationPath /= "bittorrent";
    fs::create_directory(destinationPath);
    destinationPath /= newFileName;
    lockFileNames();
    if(fileNames.find(newFileName.string()) != fileNames.end())
        return false;
    fileNames.insert(newFileName);
    //FIXME if file with same name already exists - leaves the old one quietly
    if(!fs::exists(destinationPath))
       fs::copy(fromPath, destinationPath);
    unlockFileNames();
    return true;
}
