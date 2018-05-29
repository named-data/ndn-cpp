analog-reading example application for Arduino
==============================================

These are instructions to build analog-reading, an example application for Arduino.

(These instructions may work for systems other than the following, but haven't been tested.)

## Ubuntu 14.04

If the Java JDK is not installed, enter:

    sudo apt install openjdk-7-jdk

## Ubuntu 16.04 and 18.04

If the Java JDK is not installed, enter:

    sudo apt install openjdk-8-jdk-headless

## Both Ubuntu 14.04, 16.04 and 18.04

Enter the following so that ./configure will run:

    sudo apt install build-essential libssl-dev

In the following, `<NDN-CPP root>` is the root of the NDN-CPP distribution.
Enter:

    cd <NDN-CPP root>
    ./configure

Arduino does not have some header files, so edit `<NDN-CPP root>/include/ndn-cpp/ndn-cpp-config.h`
and change for following lines:

    #define NDN_CPP_HAVE_MEMORY_H 1
    #define NDN_CPP_HAVE_GETTIMEOFDAY 1
    #define NDN_CPP_HAVE_GMTIME_SUPPORT 1
    #define NDN_CPP_HAVE_SYS_TIME_H 1

to

    #define NDN_CPP_HAVE_MEMORY_H 0
    #define NDN_CPP_HAVE_GETTIMEOFDAY 0
    #define NDN_CPP_HAVE_GMTIME_SUPPORT 0
    #define NDN_CPP_HAVE_SYS_TIME_H 0

Download and uncompress the Arduino IDE from http://www.arduino.cc/en/Main/Software .
In the following, `<ARDUINO>` is the Arduino directory.
The following is a simple way to get the NDN-CPP public include directory in the
Arduino build path. Change to the directory `<ARDUINO>/hardware/tools/avr/avr/include`
and enter:

    ln -s <NDN-CPP root>/include/ndn-cpp

The declaration of atexit() defined in Arduino.h conflicts with the one defined in stdlib.h. Edit
`<ARDUINO>/hardware/arduino/avr/cores/arduino/Arduino.h` and change the following line:

    int atexit(void (*func)()) __attribute__((weak));

to

    int atexit(void (*func)(void)) __attribute__((weak));

Enter the following to start the Arduino IDE:

    <ARDUINO>/arduino &

Click the menu File >> Open and from the NDN-CPP root select 
`<NDN-CPP root>/examples/arduino/analog-reading/analog-reading.ino` .
In the tab ndn_cpp_root.h, change "/please/fix/NDN_CPP_ROOT/in/ndn_cpp_root.h" to
the path up to the NDN-CPP root. For example, `/home/myuser` .
To compile, click the menu Sketch >> Verify/Compile.
