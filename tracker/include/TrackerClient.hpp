#pragma once
#include "Config.hpp"
#include <string>
#include <set>
#include <map>

using DataAndIp = std::pair<Config::Data, std::string>;

class TrackerClient {
public:
    static DataAndIp sendData(const std::string& addr, int port, const std::set<FileDescriptor>&fileNames);
};