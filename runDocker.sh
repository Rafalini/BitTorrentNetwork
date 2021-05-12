#!/bin/bash
# ./runDocker.sh [numer_of_containers]
echo "Building image... [it may take a while]"
docker build --tag my-debi .
echo "Build done, creating network..."
docker network create theNet >/dev/null
echo "Network created, starting containers..."

#run tracker container once
docker run -d -i -v $(pwd)/tracker:/tracker/ --name pc0 -p 800$i:80 my-debi 1>/dev/null
docker network connect theNet pc0 #>/dev/null
echo "Tracker container started and connected to network."

if [ $# -eq 0 ]
  then
    containers=2
  else
    containers=$1
fi

for ((i = 1; i <= $containers; i++ ));
do
  docker run -d -i -v $(pwd)/peer:/peer/ --name pc$i -p 800$i:80 my-debi 1>/dev/null
  docker network connect theNet pc$i #>/dev/null
  echo "Peer container $i started and connected to network."
done

echo "Done."
