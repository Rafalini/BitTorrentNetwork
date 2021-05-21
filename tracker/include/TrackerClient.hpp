#pragma once

#include <string>
#include <set>
#include <map>

class TrackerClient {
public:
    static std::map<std::string, std::set<std::string>> sendData(const std::string& addr, int port, std::set<std::string>fileNames);
};