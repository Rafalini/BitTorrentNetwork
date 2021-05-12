FROM debian:latest
RUN apt-get -y update && apt-get -y upgrade
RUN apt-get -y install net-tools
RUN apt-get -y install vim
RUN apt-get -y install g++
RUN apt-get -y install make
RUN apt-get -y install libstdc++6
RUN apt-get -y install libboost-all-dev
