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
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Get Sources
WORKDIR /fly
COPY . /fly/

# Build (tests run separately in CI for inline debug output)
RUN cmake -E make_directory build
WORKDIR /fly/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON
RUN cmake --build . --config Debug --parallel $(nproc)
