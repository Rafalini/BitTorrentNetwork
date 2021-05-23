#include "PeerServer.hpp"

#include "TrackerClient.hpp"
#include <SocketUtils.hpp>

#include <iostream>
#include <chrono>
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
    auto [sock, addr] = createListeningServerSocket(port);
    localName = addr;
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

void PeerServer::sendHeartbeatPeriodically(const std::string& trackerAddr, int port, unsigned int interval) {
    thread([this, trackerAddr, port, interval] {
        while (running) {
            auto x = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            Config::Data newData = sendHeartbeat(trackerAddr, port);
            lockData();
            if(data != newData) {
                data = newData;
            }
            unlockData();
            this_thread::sleep_until(x);
        }
    }).detach();
}

std::map<std::string, std::set<FileDescriptor>> PeerServer::sendHeartbeat(const std::string& trackerAddr, int port) {
    auto received = TrackerClient::sendData(trackerAddr, port, localFiles);
//  Uncomment to check received data
    for (const auto &peer : received) {
        std::cout << peer.first << ": ";
        for (const auto &file : peer.second) {
            std::cout << file.filename << " - " << file.owner << ", ";
        }
        std::cout << std::endl;
    }
    return received;
}

bool PeerServer::addFile(const fs::path& fromPath) {
    fs::path newFileName = fromPath.filename();
    fs::path workingDir = fs::current_path() / "bittorrent";
    fs::create_directory(workingDir);
    fs::path destinationFile = workingDir / newFileName;
    lockLocalFiles();
    if(localFiles.find({newFileName.string(), localName}) != localFiles.end())
        return false;
    localFiles.insert({newFileName.string(), localName});
    //FIXME if file with same name already exists - leaves the old one quietly, what about file with same name, but different owners
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
