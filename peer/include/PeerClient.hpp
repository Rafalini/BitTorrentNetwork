#pragma once
#include "Config.hpp"
#include "PeerServer.hpp"
#include <string>
#include <set>

class PeerClient {
public:
    static PeerServer::DownloadResult startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file);
};