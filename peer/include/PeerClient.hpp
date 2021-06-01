#pragma once
#include "Config.hpp"
#include "PeerServer.hpp"
#include <string>
#include <set>

namespace fs = std::filesystem;

class PeerClient {
    static long bytesAlreadyOwned(std::filesystem::path file);
    static void downloadNBytes(int socket, long bytesToDownload, fs::path destinationFile);
public:
    static PeerServer::DownloadResult startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file);
};