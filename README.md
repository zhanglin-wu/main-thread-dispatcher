# Main Thread Dispatcher with Glibmm on Linux

## Overview

The objective of these C++ examples is to demonstrate how to use the Glibmm classes to dispatch tasks into the main thread on Linux.

## Build and Run

#### Linux

```
git clone https://github.com/zhanglin-wu/main-thread-dispatcher

cd examples

g++ -g -Wall -o main-thread-dispatcher main-thread-dispatcher.cpp `pkg-config --cflags --libs glibmm-2.4 giomm-2.4`

./main-thread-dispatcher
```

#### Docker container (Host: Windows/macOS/Linux)

##### Prerequisites

- [Docker](https://docs.docker.com/get-docker/)
- [Visual Studio Code](https://code.visualstudio.com/)

- [VS Code Extension: Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers), for connecting with a docker container
- [VS Code Extension: C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools), for editing and debugging C++ code

##### Build and debug C++ program in docker

Open the folder in VS Code, execute the command "Remote-Containers: OpenFolder in Container...", and then wait for it to build and run the docker image.

Once you are inside the docker container, you can use the commans that you would use on a Linux system to build and run the examples.

See more details about how to run C++ programs in a docker container at: [cairomm-pangomm-on-linux-docker](https://github.com/zhanglin-wu/cairomm-pangomm-on-linux-docker).

### References

- [Using Glib::Dispatcher](https://developer.gnome.org/gtkmm-tutorial/stable/sec-using-glib-dispatcher.html.en)
- [Glib::Dispatcher: Inter-thread communication](https://developer.gnome.org/glibmm/2.64/classGlib_1_1Dispatcher.html)
