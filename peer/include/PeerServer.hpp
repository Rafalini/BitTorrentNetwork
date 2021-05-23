#pragma once
#include "Config.hpp"
#include <string>
#include <set>
#include <map>
#include <mutex>
#include <memory>
#include <ostream>
#include <filesystem>

class PeerServer {
private:
    PeerServer();
public:
    static PeerServer* instance();
    void listenAndServe(const std::string& addr, int port);
    Config::Data sendHeartbeat(const std::string& trackerAddr, int port);
    void stop() { running = false;}
    std::set<FileDescriptor>& getLocalFiles(){ return localFiles; }
    const Config::Data& getData() { return data; }
    //synchronization methods
    void lockData();
    void unlockData();
    void lockLocalFiles();
    void unlockLocalFiles();
    bool addFile(const std::filesystem::path &fromPath);
private:
    Config::Data data;
    std::set<FileDescriptor> localFiles;
    void sendHeartbeatPeriodically(const std::string &trackerAddr, int port, unsigned int interval);
    bool running = true;
    std::mutex fileNamesMutex;
    std::mutex dataMutex;
    static std::mutex singletonMutex;
    static std::unique_ptr<PeerServer> peerServer;
};