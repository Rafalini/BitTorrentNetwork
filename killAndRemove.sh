#!/bin/bash
echo "Killing all these moth*uckers:"
docker kill $(docker ps -q) 2>/dev/null
docker rm $(docker ps -a -q) 2>/dev/null
docker network rm theNet 2>/dev/null
