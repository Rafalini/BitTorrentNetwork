#include "Functions.hpp"
#include <fstream>
#include <string>

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

std::vector<std::string> getFileList(std::string json)
{
  std::vector<std::string> fileList;
  std::stringstream ss;
  ss<<json.c_str();

  pt::ptree root;
  pt::read_json(ss,root);
  fileList.push_back(root.get<std::string>("message"));

  for (pt::ptree::value_type &file : root.get_child("filesList"))
      fileList.push_back(file.second.data());

  return fileList;
}


void checkError(int err, const char *msg)
{
  if(err<0)
  {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

void updatePeerList()
{

}
