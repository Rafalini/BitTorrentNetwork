#pragma once
#include "Config.hpp"
#include "PeerServer.hpp"
#include <string>
#include <set>

class PeerClient {
    static long bytesAlreadyOwned(std::filesystem::path file);
public:
    static PeerServer::DownloadResult startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file);
};