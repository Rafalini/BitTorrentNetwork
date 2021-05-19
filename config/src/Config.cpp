#include "Config.hpp"

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
        for (pt::ptree::value_type &setItem : tree.get_child(mapItem.first))
            val.insert(setItem.second.data());

        cfg.insert({mapItem.first, val});
    }

    return cfg;
}

pt::ptree Config::generatePropertyTree(const Data& cfg) {
    pt::ptree tree;
    for (const auto& mapItem : cfg) {
        pt::ptree set;
        for (const auto& setItem : mapItem.second) {
            pt::ptree setNode;
            setNode.put("", setItem);

            set.push_back(std::make_pair("", setNode));
        }

        tree.add_child(mapItem.first, set);
    }

    return tree;
}
