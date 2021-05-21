#include "TrackerServer.hpp"

#include <SocketUtils.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

[[noreturn]] void TrackerServer::listenAndServe(const std::string &configName, int port) {
    loadConfig(configName);

    // create socket file descriptor
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        throw std::runtime_error("couldn't create socket file descriptor");

    // set socket options
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("couldn't set socket options");

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // binding socket to specific port
    if (bind(sock, (struct sockaddr *) &address, sizeof(address)) == -1)
        throw std::runtime_error("couldn't bind socket to specified port");

    // change socket mode to listening
    if (listen(sock, 3))
        throw std::runtime_error("couldn't change socket mode to listening");

    while (true) {
        struct sockaddr_in client = {0};
        unsigned int size = sizeof(client);

        int msgSocket = accept(sock, (struct sockaddr *) &client, &size);
        if (msgSocket == -1)
            std::cerr << "couldn't accept connection from peer";

        std::string clientIP = inet_ntoa(client.sin_addr);
        handleRequest(msgSocket, clientIP, configName);

        close(msgSocket);
    }
}

void TrackerServer::handleRequest(int msgSocket, const std::string &clientIP, const std::string &configName) {
    std::string request = readMsg(msgSocket);

    auto peerFiles = Config::generatePeerSet(request, ' ');
    updateConfig(configName, clientIP, peerFiles);

    auto strCfg = Config::generateStringConfig(cfg);
    sendMsg(msgSocket, strCfg);
}

void TrackerServer::updateConfig(const std::string& configName, const std::string& peerIP, const std::set<std::string>& peerFiles) {
    cfg.insert({peerIP, peerFiles});
    Config::save(configName, cfg);
}

void TrackerServer::loadConfig(const std::string &configName) {
    try {
        cfg = Config::load(configName);
    } catch (boost::wrapexcept<boost::property_tree::json_parser::json_parser_error> &ex) {
        cfg = {};
    }
}
