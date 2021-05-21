#pragma once

#include <string>
#include <set>
#include <map>

class Peer {
public:
    void listenAndServe(const std::string& addr, int port);
    std::map<std::string, std::set<std::string>> sendHeartbeat(const std::string& trackerAddr, int port);

private:
    std::set<std::string> fileNames = {"file1", "file2", "file3"};

    void sendHeartbeatPeriodically(const std::string &trackerAddr, int port, unsigned int interval);
};