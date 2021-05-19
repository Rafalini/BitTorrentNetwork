#include "Config.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

BOOST_AUTO_TEST_SUITE(ConfigTests)

    BOOST_AUTO_TEST_CASE(generate_empty_config) {
        pt::ptree tree;
        std::istringstream in("{}");

        pt::read_json(in, tree);
        auto cfg = Config::generateConfig(tree);

        auto expected = Config::Data{};
        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(single_key_config) {
        pt::ptree tree;
        std::istringstream in(""
                              "{"
                              "   \"peer1\" : [\"file1\", \"file2\", \"file3\"]"
                              "}");

        pt::read_json(in, tree);
        auto cfg = Config::generateConfig(tree);

        auto expected = Config::Data{
                {"peer1", {"file1", "file2", "file3"}}
        };
        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(multiple_keys_config) {
        pt::ptree tree;
        std::istringstream in(""
                              "{"
                              "   \"peer1\" : [\"file1\", \"file2\", \"file3\"],"
                              "   \"peer2\" : [\"file2\"],"
                              "   \"peer3\" : [\"file1\", \"file3\", \"file2\", \"file4\"],"
                              "   \"peer4\" : [\"file2\", \"file4\"]"
                              "}");

        pt::read_json(in, tree);
        auto cfg = Config::generateConfig(tree);

        auto expected = Config::Data{
                {"peer1", {"file1", "file2", "file3"}},
                {"peer2", {"file2"}},
                {"peer3", {"file1", "file3", "file2", "file4"}},
                {"peer4", {"file2", "file4"}}
        };
        BOOST_TEST(cfg == expected);
    }

    BOOST_AUTO_TEST_CASE(empty_property_tree) {
        auto cfg = Config::Data{};
        auto tree = Config::generatePropertyTree(cfg);

        pt::ptree expected;
        std::istringstream in("{}");

        pt::read_json(in, expected);
        BOOST_TEST(tree == expected);
    }

    BOOST_AUTO_TEST_CASE(single_key_property_tree) {
        auto cfg = Config::Data{
                {"peer1", {"file1", "file2", "file3"}}
        };
        auto tree = Config::generatePropertyTree(cfg);

        pt::ptree expected;
        std::istringstream in("{"
                              "\"peer1\": [\"file1\", \"file2\", \"file3\"]"
                              "}");

        std::cout << "\"" <<  tree.data() << "\"";

        pt::read_json(in, expected);
        BOOST_TEST(tree == expected);
    }

    BOOST_AUTO_TEST_CASE(multiple_keys_property_tree) {
        auto cfg = Config::Data{
                {"peer1", {"file1", "file2", "file3"}},
                {"peer2", {"file2"}},
                {"peer3", {"file1", "file3", "file2", "file4"}},
                {"peer4", {"file2", "file4"}}
        };
        auto tree = Config::generatePropertyTree(cfg);

        pt::ptree expected;
        std::istringstream in("{"
                              "\"peer1\": [\"file1\", \"file2\", \"file3\"],"
                              "\"peer2\": [\"file2\"],"
                              "\"peer3\": [\"file1\", \"file2\", \"file3\", \"file4\"],"
                              "\"peer4\": [\"file2\", \"file4\"]"
                              "}");

        pt::read_json(in, expected);
        BOOST_TEST(tree == expected);
    }

BOOST_AUTO_TEST_SUITE_END()