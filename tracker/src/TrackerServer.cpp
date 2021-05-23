#include "TrackerServer.hpp"

#include <SocketUtils.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>

[[noreturn]] void TrackerServer::listenAndServe(const std::string &configName, int port) {
    loadConfig(configName);

    auto sock = createListeningServerSocket(port);
    while (true) {
        struct sockaddr_in client = {0};
        unsigned int size = sizeof(client);
        int msgSocket = accept(sock, (struct sockaddr *) &client, &size);
        if (msgSocket == -1)
            std::cerr << "couldn't accept connection from peer";

        std::thread([this, msgSocket, client, configName] {
            std::string clientIP = inet_ntoa(client.sin_addr);
            handleRequest(msgSocket, clientIP, configName);
            close(msgSocket);
        }).detach();
    }
}

void TrackerServer::handleRequest(int msgSocket, const std::string &clientIP, const std::string &configName) {
    std::string request = readMsg(msgSocket);
    auto peerFiles = Config::decodePeerSetMsg(request);

    std::lock_guard<std::mutex> guard(cfgMutex);
    updateConfig(configName, clientIP, peerFiles);

    //send ip addr
    sendMsg(msgSocket, clientIP);

    auto strCfg = Config::encodeConfig(cfg);
    sendMsg(msgSocket, strCfg);
}

void TrackerServer::updateConfig(const std::string& configName, const std::string& peerIP, const std::set<FileDescriptor>& peerFiles) {
    cfg[peerIP] = peerFiles;
    Config::save(configName, cfg);
}

void TrackerServer::loadConfig(const std::string &configName) {
    try {
        cfg = Config::load(configName);
    } catch (boost::wrapexcept<boost::property_tree::json_parser::json_parser_error> &ex) {
        cfg = {};
    }
}
