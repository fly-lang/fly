FROM ubuntu:latest AS build
MAINTAINER <dev@flylang.org>

# Configure System
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get update && apt-get install -y build-essential libxml2-dev libtinfo-dev git pip binutils-dev libdw-dev libdwarf-dev
RUN pip install cmake --upgrade

# Get Sources
WORKDIR /fly
COPY . /fly/

# Start Build
RUN cmake -E make_directory build
WORKDIR /fly/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
RUN cmake --build . --config Debug
RUN ctest -C Release --rerun-failed --output-on-failure
