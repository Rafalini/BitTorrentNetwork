#pragma once

#include <string>

void sendMsg(int socket, const std::string& msg);

void sendXBytes(int socket, unsigned int x, void *buffer);

std::string readMsg(int socket);

void readXBytes(int socket, unsigned int x, void *buffer);