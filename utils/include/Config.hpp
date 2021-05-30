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
        return filename < other.filename || (filename == other.filename && owner < other.owner);
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
    using path = pt::ptree::path_type;

    static Data load(const std::string &filename);
    static void save(const std::string &filename, const Data& cfg);

    static std::string encodePeerSet(const std::set<FileDescriptor> &files);
    static std::string encodePeerSetMsg(const std::set<FileDescriptor> &files);
    static std::set<FileDescriptor> decodePeerSetMsg(const std::string &msg);

    static std::string encodeConfig(const Data &cfg);
    static Data decodeConfig(const std::string &msg);
    static Data decodePropertyTree(boost::property_tree::ptree tree);
};