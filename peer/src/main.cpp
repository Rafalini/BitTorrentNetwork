#include "PeerServer.hpp"

int main(int argc, char const *argv[])
{
    PeerServer peer;
    peer.listenAndServe("192.168.20.10", 8080);
}
