#include "TrackerClient.hpp"

#include "SocketUtils.hpp"

#include <unistd.h>
#include <Config.hpp>

std::map<std::string, std::set<std::string>> TrackerClient::sendData(const std::string& addr, int port, const std::set<std::string>& fileNames) {
    int sock = createListeningClientSocket(addr, port);

    std::string message;
    for (const auto& filename : fileNames)
        message += filename + " ";
    message.pop_back(); // remove leading space

    sendMsg(sock, message);

    // get response
    std::string response = readMsg(sock);

    // FIXME trim garbage
    std::size_t found = response.find_last_of('}');
    response = response.substr(0,found + 1);

    auto cfg = Config::generateConfig(response);

    close(sock);

    return cfg;
}
