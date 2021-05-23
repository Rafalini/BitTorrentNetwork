#include "Config.hpp"

#include <boost/property_tree/ini_parser.hpp>
#include <iostream>

Config::Data Config::load(const std::string &filename) {
    pt::ptree tree;
    pt::read_json(filename, tree);

    return generateConfig(tree);
}

void Config::save(const std::string &filename, const Data& cfg) {
    auto tree = generatePropertyTree(cfg);
    pt::write_json(filename, tree);
}

Config::Data Config::generateConfig(pt::ptree tree) {
    Data cfg;
    for (pt::ptree::value_type &mapItem : tree) {
        std::set<FileDescriptor> val;
        typedef pt::ptree::path_type path;
        for (pt::ptree::value_type &setItem : tree.get_child(path(mapItem.first, ' ')))
            val.insert(FileDescriptor{setItem.second.data(), "dummy OWNER"});

        cfg[mapItem.first] = val;
    }

    return cfg;
}

pt::ptree Config::generatePropertyTree(const Data& cfg) {
    pt::ptree tree;
    for (const auto& mapItem : cfg) {
        pt::ptree set;
        for (const auto& setItem : mapItem.second) {
            pt::ptree file;
            file.push_back(pt::ptree::value_type("filename", setItem.filename));
            file.push_back(pt::ptree::value_type("owner", setItem.owner));
            set.push_back(pt::ptree::value_type("", file));
        }
        tree.push_back(pt::ptree::value_type(mapItem.first, set));
    }
    return tree;
}

std::string Config::generateStringConfig(const Data& cfg) {
    auto pTree = generatePropertyTree(cfg);

    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, pTree);
    return ss.str();
}

Config::Data Config::generateConfig(const std::string& strCfg) {
    pt::ptree tree;
    std::istringstream in(strCfg);

    pt::read_json(in, tree);
    return generateConfig(tree);
}

std::set<FileDescriptor> Config::generatePeerSet(const std::string &msg, char delimiter) {
    std::string line;
    std::set<FileDescriptor> peerList;
    std::stringstream ss(msg);

    while (std::getline(ss, line, delimiter))
        peerList.insert({line});

    return peerList;
}
