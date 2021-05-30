#include "TrackerServer.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
    TrackerServer tracker;
    std::cout << "Tracker created!"<<std::endl;
    tracker.listenAndServe("cfg", 8080);
}
