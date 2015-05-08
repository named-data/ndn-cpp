analog-reading example application for Arduino
==============================================

These are instructions to build analog-reading, an example application for Arduino.

(These instructions may work for systems other than Ubuntu 14.04, but haven't been tested.)

## Ubuntu 14.04 (64 bit)

If the Java JDK is not installed, enter:

    sudo apt-get install openjdk-7-jdk

Enter the following install so that ./configure will run:

    sudo apt-get install build-essential libssl-dev

In the following, <NDN-CPP root> is the root of the NDN-CPP distribution.
Enter:

    cd <NDN-CPP root>
    ./configure

Arduino does not have memory.h, so edit <NDN-CPP root>/include/ndn-cpp/ndn-cpp-config.h
and change:

    #define NDN_CPP_HAVE_MEMORY_H 1

to

    #define NDN_CPP_HAVE_MEMORY_H 0

Download and uncompress the Arduino IDE from http://www.arduino.cc/en/Main/Software .
In the following <ARDUINO> is the Arduino directory.
The following is a simple way to get the NDN-CPP public include directory in the
Arduino build path. Change to the directory <ARDUINO>/hardware/tools/avr/avr/include
and enter:

    ln -s <NDN-CPP root>/include/ndn-cpp

Enter the following to start the Arduino IDE:

    <ARDUINO>/arduino &

Click the menu File >> Open and from the NDN-CPP root select 
examples/arduino/analog-reading/analog-reading.ino .
In the tab ndn_cpp_root.h, change "/please/fix/NDN_CPP_ROOT/in/ndn_cpp_root.h" to
the path up to the NDN-CPP root. For example, "/home/myuser" .
To compile, click the menu Sketch >> Verify/Compile.
