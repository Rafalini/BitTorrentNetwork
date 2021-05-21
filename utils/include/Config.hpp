#pragma once

#include <string>
#include <utility>
#include <map>
#include <set>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

class Config {
public:
    typedef std::map<std::string, std::set<std::string>> Data;

    static Data load(const std::string &filename);
    static void save(const std::string &filename, const Data& cfg);

    static Data generateConfig(pt::ptree tree);
    static Data generateConfig(const std::string& cfg);

    static std::string generateStringConfig(const Data& cfg);
    static pt::ptree generatePropertyTree(const Data& cfg);
    static std::set<std::string> generatePeerSet(const std::string &msg, char delimiter);
};