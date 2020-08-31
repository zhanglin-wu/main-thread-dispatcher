FROM ubuntu:16.04

# Config apt and install dev tools
RUN apt-get update -y && \
    # Install C++ tools
    apt-get install -y build-essential cmake cppcheck valgrind

# Install dependencies
RUN apt-get install -y libglibmm-2.4-dev

# Clean up
# RUN apt-get autoremove -y && \
#   apt-get clean -y && \
#   rm -rf /var/lib/apt/lists/*