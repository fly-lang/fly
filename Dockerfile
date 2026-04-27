FROM ubuntu:latest AS build
LABEL maintainer="dev@flylang.org"

# Configure System
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libxml2-dev \
    libtinfo-dev \
    libzstd-dev \
    git \
    binutils-dev \
    libdw-dev \
    libdwarf-dev \
    && rm -rf /var/lib/apt/lists/*

# Get Sources
WORKDIR /fly
COPY . /fly/

# Start Build
RUN cmake -E make_directory build
WORKDIR /fly/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
RUN cmake --build . --config Debug --parallel $(nproc)
RUN ctest -C Debug --rerun-failed --output-on-failure
