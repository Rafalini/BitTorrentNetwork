#include "../include/SocketUtils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define MAX_MSG_SIZE 1 << 14

void sendMsg(int socket, const std::string& msg) {
    unsigned int msgSize = msg.size();
    // send header
    sendXBytes(socket, sizeof(msgSize), &msgSize);

    // send message
    sendXBytes(socket, msgSize, (void *) msg.c_str());
}

void sendXBytes(int socket, unsigned int x, void *buffer) {
    int sendBytes = 0;
    while (sendBytes < x) {
        int packageSize = send(socket, buffer + sendBytes, x - sendBytes, 0);
        if (packageSize <= 0)
            std::cerr << "couldn't send bytes when expected";

        sendBytes += packageSize;
    }
}

std::string readMsg(int socket) {
    // read header with message size
    unsigned int msgLength = 0;

    readXBytes(socket, sizeof(msgLength), (void *) (&msgLength));

    if (msgLength > MAX_MSG_SIZE)
        std::cerr << "can't send message because it's too long";

    std::string buffer;
    buffer.resize(msgLength, 0);
    readXBytes(socket, msgLength, (void *)buffer.data());

    return buffer;
}

void readXBytes(int socket, unsigned int x, void *buffer) {
    int bytesRead = 0;
    while (bytesRead < x)
    {
        int result; result = read(socket, buffer + bytesRead, x - bytesRead);
        if (result < 1) {
            std::cerr << "couldn't read bytes when expected";
            return;
        }
        bytesRead += result;
    }
}

int createListeningServerSocket(int port) {
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

    return sock;
}

int createListeningClientSocket(const std::string& addr, int port) {
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

    return sock;
}