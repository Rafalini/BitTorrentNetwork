#pragma once
#include "Config.hpp"
#include "TrackerClient.hpp"
#include <string>
#include <set>
#include <map>
#include <mutex>
#include <memory>
#include <ostream>
#include <filesystem>


class PeerServer {
private:
    PeerServer();

public:
    static PeerServer *instance();

    void listenAndServe(const std::string &addr, int port);

    DataAndIp sendHeartbeat(const std::string &trackerAddr, int port);

    void stop() { running = false; }

    std::set<FileDescriptor> &getLocalFiles() { return localFiles; }

    const Config::Data &getData() { return data; }

    //synchronization methods
    void lockData();

    void unlockData();

    void lockLocalFiles();

    void unlockLocalFiles();

    std::map<FileDescriptor, std::set<std::string>> transformData(const Config::Data &);

    enum class DownloadResult {
        DOWNLOAD_OK = 0,
        FILE_NOT_FOUND,
        FILE_ALREADY_PRESENT
    };
    DownloadResult downloadFile(const std::string& fileName, const std::string& owner);
    void updateData(const Config::Data& data);
    bool addFile(const std::filesystem::path &fromPath);
    std::string getMyAddr();
private:
    std::string myAddr;
    int chunkSize = 1024; //bytes
    void startDownloadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file);
    void startUploadingFile(const std::pair<FileDescriptor, std::set<std::string>>& file);
    std::string localName = "localhost";
    Config::Data data;
    std::set<FileDescriptor> localFiles;
    void sendHeartbeatPeriodically(const std::string &trackerAddr, int port, unsigned int interval);
    bool running = true;
    std::mutex fileNamesMutex;
    std::mutex dataMutex;
    static std::mutex singletonMutex;
    static std::unique_ptr<PeerServer> peerServer;
};