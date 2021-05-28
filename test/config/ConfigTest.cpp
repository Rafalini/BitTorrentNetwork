#include "Config.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/property_tree/ptree.hpp>

BOOST_AUTO_TEST_SUITE(ConfigTests)

    BOOST_AUTO_TEST_CASE(decode_empty_set) {
        pt::ptree tree;
        std::string in(R"({"files" : []})");

        auto cfg = Config::decodePeerSetMsg(in);
        auto expected = std::set<FileDescriptor>{};

        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(decode_one_element_set) {
        pt::ptree tree;
        std::string in(R"({"files" : [{"filename" : "file1", "owner" : "192.168.20.11"}]})");

        auto cfg = Config::decodePeerSetMsg(in);
        auto expected = std::set<FileDescriptor>{FileDescriptor{"file1", "192.168.20.11"}};

        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(decode_multiple_element_set) {
        pt::ptree tree;
        std::string in(R"({"files" : [{"filename" : "file1", "owner" : "192.168.20.11"}, {"filename" : "file2", "owner" : "192.168.20.12"},
        {"filename" : "file3", "owner" : "192.168.20.11"}, {"filename" : "file4", "owner" : "192.168.20.13"}]})");

        auto cfg = Config::decodePeerSetMsg(in);
        auto expected = std::set<FileDescriptor>{FileDescriptor{"file1", "192.168.20.11"}, FileDescriptor{"file2", "192.168.20.12"},
                                                 FileDescriptor{"file3", "192.168.20.11"}, FileDescriptor{"file4", "192.168.20.13"}};

        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(encode_empty_set) {
        auto cfg = std::set<FileDescriptor>{};

        auto encodedCfg = Config::encodePeerSetMsg(cfg);
        std::string expected(R"({"files" : []})");

        BOOST_TEST(encodedCfg == expected);
    }

    BOOST_AUTO_TEST_CASE(encode_one_element_set) {
        auto cfg = std::set<FileDescriptor>{FileDescriptor{"file1", "192.168.20.11"}};

        auto encodedCfg = Config::encodePeerSetMsg(cfg);
        std::string expected(R"({"files" : [{"filename" : "file1", "owner" : "192.168.20.11"}]})");

        BOOST_TEST(encodedCfg == expected);
    }

    BOOST_AUTO_TEST_CASE(encode_multiple_element_set) {
        auto cfg = std::set<FileDescriptor>{FileDescriptor{"file1", "192.168.20.11"}, FileDescriptor{"file2", "192.168.20.12"},
                                                 FileDescriptor{"file3", "192.168.20.11"}, FileDescriptor{"file4", "192.168.20.13"}};

        auto encodedCfg = Config::encodePeerSetMsg(cfg);
        std::string expected(R"({"files" : [{"filename" : "file1", "owner" : "192.168.20.11"},{"filename" : "file2", "owner" : "192.168.20.12"},{"filename" : "file3", "owner" : "192.168.20.11"},{"filename" : "file4", "owner" : "192.168.20.13"}]})");

        BOOST_TEST(encodedCfg == expected);
    }

    BOOST_AUTO_TEST_CASE(decode_empty_map) {
        std::string in("{}");

        auto cfg = Config::decodeConfig(in);

        auto expected = Config::Data{};
        BOOST_TEST(cfg == expected);
    }


    BOOST_AUTO_TEST_CASE(decode_single_key_map) {
        std::string in(R"({"192.168.20.11" : [{"filename" : "file1", "owner" : "192.168.20.11"}, {"filename" : "file2", "owner" : "192.168.20.12"}, {"filename" : "file3", "owner" : "192.168.20.12"}]})");

        auto cfg = Config::decodeConfig(in);
        auto expected = Config::Data {
                {"192.168.20.11", {FileDescriptor{"file1", "192.168.20.11"}, FileDescriptor{"file2", "192.168.20.12"},
                                          FileDescriptor{"file3", "192.168.20.12"}}},
        };

        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(decode_multiple_keys_map) {
        pt::ptree tree;

        std::string peerList1(R"("192.168.20.11" : [{"filename" : "file1", "owner" : "192.168.20.11"}, {"filename" : "file2", "owner" : "192.168.20.12"}, {"filename" : "file3", "owner" : "192.168.20.12"}])");
        std::string peerList2(R"("192.168.20.12" : [{"filename" : "file3", "owner" : "192.168.20.12"}])");
        std::string peerList3(R"("192.168.20.13" : [])");
        std::string in("{" + peerList1 + "," + peerList2 + "," + peerList3 + "}");

        auto cfg = Config::decodeConfig(in);
        auto expected = Config::Data{
                {"192.168.20.11", {FileDescriptor{"file1", "192.168.20.11"}, FileDescriptor{"file2", "192.168.20.12"}, FileDescriptor{"file3", "192.168.20.12"}}},
                {"192.168.20.12", {FileDescriptor{"file3", "192.168.20.12"}}},
                {"192.168.20.13", {}},
        };

        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(encode_empty_map) {
        auto cfg = Config::Data{};

        auto encodedCfg = Config::encodeConfig(cfg);
        std::string expected("{}");

        BOOST_TEST(encodedCfg == expected);
    }

    BOOST_AUTO_TEST_CASE(encode_single_key_map) {
        auto cfg = Config::Data {
                {"192.168.20.11", {FileDescriptor{"file1", "192.168.20.11"}, FileDescriptor{"file2", "192.168.20.12"},
                                          FileDescriptor{"file3", "192.168.20.12"}}},
        };

        auto encodedCfg = Config::encodeConfig(cfg);
        std::string expected(R"({"192.168.20.11" : [{"filename" : "file1", "owner" : "192.168.20.11"},{"filename" : "file2", "owner" : "192.168.20.12"},{"filename" : "file3", "owner" : "192.168.20.12"}]})");

        BOOST_TEST(encodedCfg == expected);
    }

    BOOST_AUTO_TEST_CASE(encode_multiple_keys_map) {
        auto cfg = Config::Data{
                {"192.168.20.11", {FileDescriptor{"file1", "192.168.20.11"}, FileDescriptor{"file2", "192.168.20.12"}, FileDescriptor{"file3", "192.168.20.12"}}},
                {"192.168.20.12", {FileDescriptor{"file3", "192.168.20.12"}}},
                {"192.168.20.13", {}},
        };

        auto encodedCfg = Config::encodeConfig(cfg);

        std::string peerList1(R"("192.168.20.11" : [{"filename" : "file1", "owner" : "192.168.20.11"},{"filename" : "file2", "owner" : "192.168.20.12"},{"filename" : "file3", "owner" : "192.168.20.12"}])");
        std::string peerList2(R"("192.168.20.12" : [{"filename" : "file3", "owner" : "192.168.20.12"}])");
        std::string peerList3(R"("192.168.20.13" : [])");
        std::string expected("{" + peerList1 + "," + peerList2 + "," + peerList3 + "}");


        BOOST_TEST(encodedCfg == expected);
    }

BOOST_AUTO_TEST_SUITE_END()