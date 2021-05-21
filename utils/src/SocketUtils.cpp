#include "../include/SocketUtils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define MAX_MSG_SIZE 1 << 14

void sendMsg(int socket, const std::string& msg) {
    unsigned int msgSize = msg.size();
    char *header = new char[sizeof(msgSize)];
    memcpy(header, (void *) &msgSize, sizeof(msgSize));

    // send header
    sendXBytes(socket, sizeof(msgSize), header);

    // send message
    sendXBytes(socket, msgSize, (void *) msg.c_str());

    delete []header;
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

    // read message
    char *buffer = new char[msgLength]; // not using smart pointers since it's not possible to do the trick with casting on void*
    readXBytes(socket, msgLength, (void *) buffer);

    std::string request(buffer);

    delete[]buffer;

    return request;
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
