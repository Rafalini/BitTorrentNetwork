#include "Config.hpp"

#include <boost/property_tree/ini_parser.hpp>

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
        std::set<std::string> val;
        typedef pt::ptree::path_type path;
        for (pt::ptree::value_type &setItem : tree.get_child(path(mapItem.first, ' ')))
            val.insert(setItem.second.data());

        cfg.insert({mapItem.first, val});
    }

    return cfg;
}

pt::ptree Config::generatePropertyTree(const Data& cfg) {
    pt::ptree tree;
    for (const auto& mapItem : cfg) {
        pt::ptree set;
        for (const auto& setItem : mapItem.second)
            set.push_back(pt::ptree::value_type("", setItem));

        tree.push_back(pt::ptree::value_type(mapItem.first, set));
    }

    return tree;
}

std::string Config::generateStringConfig(const Data& cfg) {
    auto ptree = generatePropertyTree(cfg);

    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, ptree);

    return ss.str();
}

Config::Data Config::generateConfig(const std::string& strCfg) {
    pt::ptree tree;
    std::istringstream in(strCfg);

    pt::read_json(in, tree);
    auto cfg = Config::generateConfig(tree);

    return generateConfig(tree);
}

std::set<std::string> Config::generatePeerSet(const std::string &msg, char delimiter) {
    std::string line;
    std::set<std::string> peerList;
    std::stringstream ss(msg);

    while (std::getline(ss, line, delimiter))
        peerList.insert(line);

    return peerList;
}
