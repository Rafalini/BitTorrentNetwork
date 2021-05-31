#include "Config.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <fstream>

Config::Data Config::load(const std::string &filename) {
    pt::ptree tree;
    pt::read_json(filename, tree);

    return decodePropertyTree(tree);
}

void Config::save(const std::string &filename, const Data& cfg) {
    std::string encodedCfg = encodeConfig(cfg);

    std::ofstream ofs(filename, std::ofstream::trunc);
    ofs << encodedCfg;
    ofs.close();
}

std::string Config::encodePeerSet(const std::set<FileDescriptor> &files) {
    std::string encodedMsg("[");
    for (auto &file : files) {
        std::string filename = R"("filename" : ")" + file.filename + "\"";
        std::string fileowner = R"("owner" : ")" + file.owner + "\"";

        encodedMsg += "{" + filename += ", "  + fileowner + "},";
    }

    if (!files.empty()) // remove leading comma
        encodedMsg.pop_back();
    encodedMsg += "]";

    return encodedMsg;
}

//enocde list of files to JSON
std::string Config::encodePeerSetMsg(const std::set<FileDescriptor> &files) {
    return "{\"files\" : " + encodePeerSet(files) + "}";
}

std::set<FileDescriptor> Config::decodePeerSetMsg(const std::string &msg) {
    pt::ptree tree;
    std::istringstream in(msg);

    pt::read_json(in, tree);

    std::set<FileDescriptor> peerList;
    for (auto &[tmp, fd] : tree.get_child("files")) {
        std::string filename = fd.get_child("filename").data();
        std::string owner = fd.get_child("owner").data();

        peerList.insert(FileDescriptor{filename, owner});
    }

    return peerList;
}
//encode to JSON
std::string Config::encodeConfig(const Config::Data &cfg) {
    std::string encodedMsg("{");
    for (auto &[peerIP, files] : cfg)
        encodedMsg +=  "\"" + peerIP + "\" : " + encodePeerSet(files) + ",";

    if (!cfg.empty()) // remove leading comma
        encodedMsg.pop_back();
    encodedMsg += "}";

    return encodedMsg;
}

//decode from JSON to property tree
Config::Data Config::decodeConfig(const std::string &msg) {
    pt::ptree tree;
    std::istringstream in(msg);

    pt::read_json(in, tree);

    return decodePropertyTree(tree);
}

Config::Data Config::decodePropertyTree(pt::ptree tree) {
    Data cfg;
    for (auto &[peerIP, tmp] : tree) {
        std::set<FileDescriptor> files;
        for (auto f : tree.get_child(path(peerIP, '|'))) {
            std::string filename = f.second.get_child("filename").data();
            std::string owner = f.second.get_child("owner").data();

            files.insert(FileDescriptor{filename, owner});
        }

        cfg.insert({peerIP, files});
    }

    return cfg;
}