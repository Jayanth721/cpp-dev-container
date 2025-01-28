# Use an official Ubuntu base image
FROM ubuntu:24.04

# Set the maintainer label (optional)
LABEL maintainer="jayanth721@gmail.com"

# Update and install build tools and C++ packages
RUN apt update && apt install -y \
    build-essential \
    gdb \
    cmake \
    git \
    curl \
    strace \
    && apt clean && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Default shell
CMD ["/bin/bash"]
