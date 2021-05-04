#!/bin/bash
# source:::::::::::::: https://docs.docker.com/engine/install/ ::::::::::::::::
sudo apt-get remove docker docker-engine docker.io containerd runc
sudo apt-get update
#install libs
sudo apt-get install apt-transport-https ca-certificates curl gnupg lsb-release
#add Docker’s official GPG key
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

#install docker
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io

#post instalation, remove upper permisions requirement
sudo groupadd docker
sudo usermod -aG docker $(whoami)
#re-log needed

#test
sudo docker run hello-world
