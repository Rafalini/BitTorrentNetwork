#include "TrackerServer.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
    std::cout << "Tracker begin..."<<std::endl;
    TrackerServer tracker;
    std::cout << "Tracker created!"<<std::endl;
    tracker.listenAndServe("cfg", 8080);
}
