#include "Peer.hpp"

int main(int argc, char const *argv[])
{
    Peer peer;
    peer.listenAndServe("192.168.20.10", 8080);
}
