#pragma once
#include "Config.hpp"
#include <string>
#include <set>
#include <map>

class TrackerClient {
public:
    static std::map<std::string, std::set<FileDescriptor>> sendData(const std::string& addr, int port, const std::set<FileDescriptor>&fileNames);
};