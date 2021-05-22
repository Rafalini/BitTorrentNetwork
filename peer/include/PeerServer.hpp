#pragma once

#include <string>
#include <set>
#include <map>

using Data = std::map<std::string, std::set<std::string>>;
class PeerServer {
public:
    void listenAndServe(const std::string& addr, int port);
    Data sendHeartbeat(const std::string& trackerAddr, int port);
    constexpr void stop() { running = false;}
    constexpr Data& getData(){ return data; }
    constexpr std::set<std::string>& getFileNames(){ return fileNames; }
private:
    Data data;
    std::set<std::string> fileNames = {"file1", "file2", "file3"};
    void sendHeartbeatPeriodically(const std::string &trackerAddr, int port, unsigned int interval);
    bool running = true;
};