#include "TrackerClient.hpp"

#include "SocketUtils.hpp"

#include <unistd.h>
#include <Config.hpp>

std::map<std::string, std::set<FileDescriptor>> TrackerClient::sendData(const std::string& addr, int port, const std::set<FileDescriptor>& fileNames) {
    int sock = createListeningClientSocket(addr, port);

    std::string message = Config::encodePeerSetMsg(fileNames);
    sendMsg(sock, message);

    // get response
    std::string response = readMsg(sock);

    auto cfg = Config::decodeConfig(response);

    close(sock);

    return cfg;
}
