#include "TrackerServer.hpp"

int main(int argc, char const *argv[]) {
    TrackerServer tracker;
    tracker.listenAndServe("cfg", 8080);
}
