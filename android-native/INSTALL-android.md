NDN-CPP: A Named Data Networking client library for C++ and C - Android
=======================================================================

This are instructions to cross-compile NDN-CPP for Android.

Prerequisites
=============

* Required: Android Studio (version 2.2 or higher)
* Required: Android LLDB, CMake and NDK

## macOS 10.12

Required: Install Android Studio from https://developer.android.com/studio/index.html .
(Tested with Android Studio 2.3.3 .)

Required: In the Android Studio SDK Manager, install LLDB, CMake and NDK following the instructions at
https://developer.android.com/ndk/guides/index.html

Build
=====

Create a new Android project with the following configuration:

* In the New Project wizard, name the project (e.g. "ndn-cpp-native"). Be sure to check "Include C++ support".
* Select a minimum SDK. This has been tested with "API 21: Android 5.0 (Lollipop)".
* Select the Basic Activity template.
* In the Customize C++ Support screen, for the C++ Standard, select "C++11". Check "Exceptions Support"
  and "Runtime Type Information Support".
* Click Finish to complete the New Project wizard.

Set up CMake as follows. In a terminal, change directory to the root of your Android Studio project,
for example "/Users/myusername/AndroidStudioProjects/ndn-cpp-native".

To make a link to NDN-CPP, in the following change <NDN-CPP-root> to the root of the NDN-CPP distribution:

    ln -s <NDN-CPP-root> app/src/ndn-cpp

The Android config.h mainly disables Boost and std shared_ptr so that it uses ndnboost included in NDN-CPP.
Copy it to the NDN-CPP include folder:

    cp app/src/ndn-cpp/android-native/ndn-cpp-config.h app/src/ndn-cpp/include/ndn-cpp

Replace the CMake file with the NDN-CPP version that makes libndn-cpp:

    cp app/src/ndn-cpp/android-native/CMakeLists.txt app

(In Android Studio, if it says "Unregistered VCS root detected", click ignore.)

in Android Studio, in the Build menu, click Rebuild Project. The libndn-cpp.so library file is in the
app/build/intermediates/cmake/debug/obj subfolder of your Android Studio project. For example if your
Android system is armeabi, then the library file is app/build/intermediates/cmake/debug/obj/armeabi/libndn-cpp.so .

## Unity on Android

To use libndn-cpp.so in your Unity on Android project, copy libndn-cpp.so to the
subfolder Assets/Plugins/Android of your Unity project folder. (Create this
folder if it doesn't exist.)
