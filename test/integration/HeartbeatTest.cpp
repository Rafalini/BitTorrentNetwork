#include <boost/test/unit_test.hpp>
#include <boost/property_tree/ptree.hpp>

#include "PeerServer.hpp"

BOOST_AUTO_TEST_SUITE(Heartbeat)

    BOOST_AUTO_TEST_CASE(SendHeartbeat) {
//        PeerServer* peer = PeerServer::instance();
//        auto res = peer->sendHeartbeat("192.168.20.10", 8080);
//
//        std::set<std::string> expectedIPs{"192.168.20.11", "192.168.20.12", "192.168.20.13"};
//        std::set<FileDescriptor> expectedFiles{{"file1", "192.168.20.11"}, {"file2", "192.168.20.11"}, {"file3", "192.168.20.11"}};
//
//        std::cout << "IPs" << std::endl;
/*        for (const auto& ip : expectedIPs) {
            bool exist = res.find(ip) != res.end();
            BOOST_ASSERT(exist);

            std::set<FileDescriptor> peerFiles = res.at(ip);
            BOOST_ASSERT(expectedFiles == peerFiles);
        }*/

    }
BOOST_AUTO_TEST_SUITE_END()