FROM debian:latest
RUN apt-get -y update && apt-get -y upgrade
RUN apt-get -y install net-tools
RUN apt-get -y install vim
