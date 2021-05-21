#include <iostream>
#include "Peer.hpp"

#include "TrackerClient.hpp"

void Peer::listenAndServe(const std::string &trackerAddr, int port) {
    auto received = sendHeartbeat(trackerAddr, port);
    for (const auto& peer : received) {
        std::cout << peer.first << ": ";
        for (const auto& file : peer.second) {
            std::cout << file << ", ";
        }
        std::cout << std::endl;
    }
}

std::map<std::string, std::set<std::string>> Peer::sendHeartbeat(const std::string &trackerAddr, int port) {
    return TrackerClient::sendData(trackerAddr, port, fileNames);
}
