#include "Peer.hpp"

#include "TrackerClient.hpp"

#include <iostream>
#include <chrono>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define HEARTBEAT_INTERVAL 30

void Peer::listenAndServe(const std::string &trackerAddr, int port) {
    sendHeartbeatPeriodically(trackerAddr, port, HEARTBEAT_INTERVAL);

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

        close(msgSocket);
    }

}

void Peer::sendHeartbeatPeriodically(const std::string& trackerAddr, int port, unsigned int interval) {
    std::thread([this, trackerAddr, port, interval] {
        while (true) {
            auto x = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            sendHeartbeat(trackerAddr, port);
            std::this_thread::sleep_until(x);
        }
    }).detach();
}

std::map<std::string, std::set<std::string>> Peer::sendHeartbeat(const std::string& trackerAddr, int port) {
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
