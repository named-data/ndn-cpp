NDN-CPP:  A Named Data Networking client library for C++ and C
==============================================================

These are instructions to build and install NDN-CPP.
To cross compile for Anrdoid, see android-native/INSTALL-android.md .

Prerequisites
=============
(These are prerequisites to build NDN-CPP.  To do development of NDN-CPP code and update the build system,
 see Development Prerequisites.)

* Required: libcrypto
* Optional: libsqlite3 (for key storage)
* Optional: OSX Security framework (for key storage)
* Optional: Protobuf (for the ProtobufTlv converter and ChronoSync)
* Optional: log4cxx (for debugging and log output)
* Optional: Doxygen (to make documentation)
* Optional: Boost (min version 1.48) with asio (for ThreadsafeFace and async I/O)

Following are the detailed steps for each platform to install the prerequisites.

## 10.9.5, OS X 10.10.2, OS X 10.11, macOS 10.12 and macOS 10.13
Install Xcode. To install the command line tools, in a terminal enter:

    xcode-select --install

Install Brew from https://brew.sh

In a terminal, enter:

    brew install openssl

Optional: To install Protobuf, in a terminal, enter:

    brew install protobuf

Optional: To install log4cxx, in a terminal, enter:

    brew install log4cxx

Optional: To install Doxygen, in a terminal, enter:

    brew install doxygen

Optional: To install Boost with asio, in a terminal, enter:

    brew install boost

## Ubuntu 12.04
In a terminal, enter:

    sudo apt-get install build-essential libssl-dev

Optional: To install libsqlite3, in a terminal, enter:

    sudo apt-get install libsqlite3-dev

Optional: To install Protobuf, in a terminal, enter:

    sudo apt-get install libprotobuf-dev protobuf-compiler

Optional: To install log4cxx, in a terminal, enter:

    sudo apt-get install liblog4cxx10-dev

Optional: To install Doxygen, in a terminal, enter:

    sudo apt-get install doxygen

Optional: To install Boost with asio, in a terminal, enter:

    sudo apt-get install libboost1.48-all-dev

## Ubuntu 14.04 and 15.04
In a terminal, enter:

    sudo apt-get install build-essential libssl-dev

Optional: To install libsqlite3, in a terminal, enter:

    sudo apt-get install libsqlite3-dev

Optional: To install Protobuf, in a terminal, enter:

    sudo apt-get install libprotobuf-dev protobuf-compiler

Optional: To install log4cxx, in a terminal, enter:

    sudo apt-get install liblog4cxx10-dev

Optional: To install Doxygen, in a terminal, enter:

    sudo apt-get install doxygen

Optional: To install Boost with asio, in a terminal, enter:

    sudo apt-get install libboost-all-dev

## Ubuntu 16.04, 16.10, 17.04 and 18.04
In a terminal, enter:

    sudo apt install build-essential libssl-dev

Optional: To install libsqlite3, in a terminal, enter:

    sudo apt install libsqlite3-dev

Optional: To install Protobuf, in a terminal, enter:

    sudo apt install libprotobuf-dev protobuf-compiler

Optional: To install log4cxx, in a terminal, enter:

    sudo apt install liblog4cxx-dev

Optional: To install Doxygen, in a terminal, enter:

    sudo apt install doxygen

Optional: To install Boost with asio, in a terminal, enter:

    sudo apt install libboost-all-dev

### Odroid Ubuntu

Optional: To use boost, you may need to manually install it since apt-get doesn't seem
to install the library files.

## Windows Cygwin
Cygwin is tested on Windows 7 64-bit.

In the Cygwin installer, select and install the "Devel" packages at the top level of the installer.
(The "Devel" packages include libcrypto and libsqlite3.)

Build
=====
(These are instructions to build NDN-CPP. To do development of NDN-CPP code and update the build system, see Development.)

To build in a terminal, change directory to the NDN-CPP root.

To configure on macOS, enter:

    ./configure ADD_CFLAGS=-I/usr/local/opt/openssl/include ADD_CXXFLAGS=-I/usr/local/opt/openssl/include ADD_LDFLAGS=-L/usr/local/opt/openssl/lib

To configure on other systems, enter:

    ./configure

Enter:

    make
    sudo make install

[Ubuntu only] Update the path to the shared libraries:

    sudo /sbin/ldconfig

To run the unit tests, in a terminal enter:

    make check

To make documentation, in a terminal enter:

    make doxygen-doc

The documentation output is in `doc/html/index.html`. (If you already did ./configure
before installing Doxygen, you need to do ./configure again before make doxygen-doc.)

Files
=====
This makes the following libraries:

* .libs/libndn-c.a: The core C code for encoding and communication.
* .libs/libndn-cpp.a: The C++ library API.  (If linking to libndn-cpp, don't link to libndn-c since it is included.)

This makes the following example programs:

* bin/test-get-async: Connect to one of the NDN testbed hubs, express an interest and display the received data.
* bin/test-publish-async-nfd: Connect to the local NFD hub, accept interests with prefix /testecho and echo back a data packet. See test-echo-consumer.
* bin/test-publish-async-nfd-lite: Use NDN-CPP Lite to connect to the local NFD hub, accept interests with prefix /testecho and echo back a data packet. See test-echo-consumer-lite.
* bin/test-echo-consumer: Prompt for a word, send the interest /testecho/word to the local hub which is echoed by test-publish-async-nfd.
* bin/test-echo-consumer-lite: Use NDN-CPP Lite to prompt for a word, send the interest /testecho/word to the local hub which is echoed by test-publish-async-nfd-lite.
* bin/test-encode-decode-interest: Encode and decode an interest, testing interest selectors and the name URI.
* bin/test-encode-decode-data: Encode and decode a data packet, including signing the data packet.
* bin/test-encode-decode-fib-entry: Encode and decode a sample Protobuf message using ProtobufTlv.
* bin/test-chrono-chat: A command-line chat application using the ChronoSync2013 API, compatible with ChronoChat-js.

Running make doxygen-doc puts code documentation in doc/html.

Development Prerequisites
=========================
These steps are only needed to do development of NDN-CPP code and update the build system.
First follow the Prerequisites above for your platform.

## OS X 10.9, OS X 10.10.2, OS X 10.11, macOS 10.12 and macOS 10.13
In a terminal, enter:

    brew install automake libtool doxygen

## Ubuntu 12.04, 14.04, 16.04 and 18.04
In a terminal, enter:

    sudo apt-get install automake libtool doxygen

Development
===========
Follow Development Prerequisites above for your platform.
Now you can add source code files and update Makefile.am.
After updating, change directory to the NDN-CPP root and enter the following to build the Makefile:

    ./autogen.sh

To build again, follow the instructions above (./configure, make, etc.)
