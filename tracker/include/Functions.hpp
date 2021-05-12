#ifndef FUNCTIONS
#define FUNCTIONS

#include <string>
#include <vector>

void addNewPeer(std::string adress);
bool ifcontains(std::string adress);
void checkError(int err, const char *msg);
void updatePeerList();
std::vector<std::string> getFileList(std::string json);

#endif
