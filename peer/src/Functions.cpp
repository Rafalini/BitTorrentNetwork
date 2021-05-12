#include "Functions.hpp"
#include <fstream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

void addNewPeer(std::string adress)
{
  if(!ifcontains(adress))
  {
    std::ofstream configFile;
    configFile.open("peerList.cfg", std::ios::out | std::ios::app | std::ios::binary);
    configFile << adress << std::endl;
    configFile.close();
  }
}

bool ifcontains(std::string adress)
{
  std::ifstream configFile ("peerList.cfg");
  std::string content;

  while(getline(configFile,content))
    if(content == adress)
      return true;
  configFile.close();
  return false;
}

void checkError(int err, const char *msg)
{
  if(err<0)
  {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

std::string getJson(std::vector<std::string> arguments, std::string message)
{
    pt::ptree root;
    pt::ptree files;
    for (std::string file : arguments)
    {
      pt::ptree fileNode;
      fileNode.put("", file);
      files.push_back(std::make_pair("",fileNode));
    }
    root.add_child("filesList", files);
    root.put("message", message.c_str());

    std::ostringstream oss;
    boost::property_tree::write_json(oss, root);
    std::string json = oss.str();
    return json;
}

void updatePeerList()
{

}
