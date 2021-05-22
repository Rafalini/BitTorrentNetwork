#pragma once

#include <string>
#include <set>
#include <map>
#include <mutex>
#include <memory>
#include <ostream>
#include <filesystem>

using Data = std::map<std::string, std::set<std::string>>;
class PeerServer {
private:
    PeerServer();
public:
    static PeerServer* instance();
    void listenAndServe(const std::string& addr, int port);
    Data sendHeartbeat(const std::string& trackerAddr, int port);
    void stop() { running = false;}
    std::set<std::string>& getFileNames(){ return fileNames; }
    const Data& getData() { return data; }
    //synchronization methods
    void lockData();
    void unlockData();
    void lockFileNames();
    void unlockFileNames();
    bool addFile(const std::filesystem::path &fromPath);
private:
    Data data;
    std::set<std::string> fileNames;
    void sendHeartbeatPeriodically(const std::string &trackerAddr, int port, unsigned int interval);
    bool running = true;
    std::mutex fileNamesMutex;
    std::mutex dataMutex;
    static std::mutex singletonMutex;
    static std::unique_ptr<PeerServer> peerServer;
};