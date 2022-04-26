#!/bin/bash

rm -rf ci/fly-src/
mkdir ci/fly-src
cp -R include/ src/ test/ CMakeLists.txt ci/fly-src
cd ci
sudo docker build -t ubuntu-fly-build .
if [ $? -eq 0 ]; then
  echo "Build Success"
else
  echo "Build error!"
fi
