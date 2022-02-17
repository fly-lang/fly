#!/bin/bash

rm -rf docker/fly-src/
mkdir docker/fly-src
cp -R include/ src/ test/ CMakeLists.txt docker/fly-src
cd docker
sudo docker build -t ubuntu-fly-build .
if [ $? -eq 0 ]; then
  echo "Build Success"
else
  echo "Build error!"
fi
