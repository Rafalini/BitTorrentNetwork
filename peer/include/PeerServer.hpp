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

namespace fs = std::filesystem;

class PeerServer {
private:
    PeerServer();

public:
    static PeerServer *instance();

    void startServer(const std::string &trackerAddr, int port);

    [[noreturn]] void listenAndServe(int port);

    DataAndIp sendHeartbeat(const std::string &trackerAddr, int port);

    void stop() { running = false; }

    std::set<FileDescriptor> &getLocalFiles() { return localFiles; }

    const Config::Data &getData() { return data; }

    //synchronization methods
    void lockData();

    void unlockData();

    void lockLocalFiles();

    void unlockLocalFiles();

    std::map<FileDescriptor, std::set<std::string>> transformData();

    enum class DownloadResult {
        FILE_REVOKED = -2,
        FILE_NOT_FOUND = -1,
        DOWNLOAD_OK = 0,
        FILE_ALREADY_PRESENT,
        DOWNLOAD_ABORTED
    };
    DownloadResult downloadFile(const std::string& fileName, const std::string& owner);
    void updateData(const Config::Data& data);
    bool addFile(const std::filesystem::path &fromPath);
    void deleteFile(const FileDescriptor& fileDescriptor);
    std::string getMyAddr();
    bool addRemoteFile(const FileDescriptor& file);
private:
    std::string myAddr;
    const int chunkSize = 1024; //size of one chunk of data that is send during file download

    FileDescriptor getDescriptor(std::string fileId);
    std::string localName = "localhost";
    Config::Data data; //std::map<std::string, std::set<FileDescriptor>>
    std::set<FileDescriptor> localFiles;

    bool running = true;
    std::mutex fileNamesMutex;
    std::mutex dataMutex;
    static std::mutex singletonMutex;
    static std::unique_ptr<PeerServer> peerServer;

    long fileSize(std::filesystem::path file);
    void uploadNBytes(int socket, long bytesToUpload, long offset, fs::path destinationFile);
    void handleDownloadRequest(int msgSocket);
    bool isFileRevoked();

    void sendHeartbeatPeriodically(const std::string &trackerAddr, int port, unsigned int interval);
};