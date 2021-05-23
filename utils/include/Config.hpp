#pragma once

#include <string>
#include <utility>
#include <map>
#include <set>
#include <ostream>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

struct FileDescriptor {
    std::string filename;
    std::string owner;
    bool operator<(const FileDescriptor& other) const {
        return filename < other.filename || filename == other.filename && owner < other.owner;
    }
    bool operator==(const FileDescriptor& other) const {
        return filename == other.filename && owner == other.owner;
    }
    std::ostream& operator<<(std::ostream& out) {
        out << "filename: " + filename + ", owner: " << owner;
        return out;
    }
};

class Config {
public:
    using Data = std::map<std::string, std::set<FileDescriptor>>;

    static Data load(const std::string &filename);
    static void save(const std::string &filename, const Data& cfg);

    static Data generateConfig(pt::ptree tree);
    static Data generateConfig(const std::string& cfg);

    static std::string generateStringConfig(const Data& cfg);
    static pt::ptree generatePropertyTree(const Data& cfg);
    static std::set<FileDescriptor> generatePeerSet(const std::string &msg, char delimiter);
private:
    std::mutex config_access_mutex;
};