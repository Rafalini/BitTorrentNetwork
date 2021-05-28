#include "TrackerClient.hpp"

#include "SocketUtils.hpp"

#include <unistd.h>
#include <Config.hpp>

DataAndIp TrackerClient::sendData(const std::string& addr, int port, const std::set<FileDescriptor>& fileNames) {
    int sock = createListeningClientSocket(addr, port);

    std::string message = Config::encodePeerSetMsg(fileNames);
    sendMsg(sock, message);

    //get ip addr
    std::string ipaddr = readMsg(sock);

    // get response
    std::string response = readMsg(sock);

    auto cfg = Config::decodeConfig(response);

    close(sock);

    return std::make_pair(cfg, ipaddr);
}
