#ifndef FUNCTIONS
#define FUNCTIONS

#include <string>
#include <vector>

void addNewPeer(std::string adress);
bool ifcontains(std::string adress);
void checkError(int err, const char *msg);
std::string getJson(std::vector<std::string> arguments, std::string message);
void updatePeerList();

#endif
