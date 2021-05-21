#pragma once

#include "Config.hpp"

#include <netinet/in.h>

#include <string>
#include <mutex>

class TrackerServer {
public:
    [[noreturn]] void listenAndServe(const std::string& configName, int port);
private:
    void handleRequest(int msgSocket, const std::string& clientIP, const std::string &configName);
    void loadConfig(const std::string& configName);
    void updateConfig(const std::string& configName, const std::string& peerIP, const std::set<std::string>& peerFiles);

    std::mutex cfgMutex;
    Config::Data cfg;
};