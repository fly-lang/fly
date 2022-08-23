#!/bin/bash

cd ..
sudo docker build -t ubuntu-fly-build .
if [ $? -eq 0 ]; then
  echo "Build Success"
else
  echo "Build error!"
fi
