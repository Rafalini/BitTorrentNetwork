#include "PeerServer.hpp"

#include "TrackerClient.hpp"
#include <SocketUtils.hpp>

#include <iostream>
#include <chrono>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define HEARTBEAT_INTERVAL 30

void PeerServer::listenAndServe(const std::string &trackerAddr, int port) {
    sendHeartbeatPeriodically(trackerAddr, port, HEARTBEAT_INTERVAL);
    std::thread([this, trackerAddr, port] {
        int sock = createListeningServerSocket(port);
        while (true) {
            struct sockaddr_in client = {0};
            unsigned int size = sizeof(client);

            int msgSocket = accept(sock, (struct sockaddr *) &client, &size);
            if (msgSocket == -1)
                std::cerr << "couldn't accept connection from peer";

            close(msgSocket);
        }
    }).detach();
}

void PeerServer::sendHeartbeatPeriodically(const std::string& trackerAddr, int port, unsigned int interval) {
    std::thread([this, trackerAddr, port, interval] {
        while (true) {
            auto x = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            sendHeartbeat(trackerAddr, port);
            std::this_thread::sleep_until(x);
        }
    }).detach();
}

std::map<std::string, std::set<std::string>> PeerServer::sendHeartbeat(const std::string& trackerAddr, int port) {
    auto received = TrackerClient::sendData(trackerAddr, port, fileNames);
    for (const auto &peer : received) {
        std::cout << peer.first << ": ";
        for (const auto &file : peer.second) {
            std::cout << file << ", ";
        }
        std::cout << std::endl;
    }

    return received;
}
