#include "TrackerClient.hpp"

#include "SocketUtils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <Config.hpp>

std::map<std::string, std::set<std::string>> TrackerClient::sendData(const std::string& addr, int port, const std::set<std::string>& fileNames) {
    // create socket file descriptor
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        throw std::runtime_error("couldn't create socket file descriptor");

    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, addr.c_str(), &server.sin_addr) == -1)
        throw std::runtime_error("invalid address");

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) == -1)
        throw std::runtime_error("couldn't connect to tracker");

    std::string message;
    for (const auto& filename : fileNames)
        message += filename + " ";
    message.pop_back(); // remove leading space

    sendMsg(sock, message);

    // get response
    std::string response = readMsg(sock);

    // trim garbage
    std::size_t found = response.find_last_of('}');
    response = response.substr(0,found + 1);

    auto cfg = Config::generateConfig(response);

    close(sock);

    return cfg;
}
