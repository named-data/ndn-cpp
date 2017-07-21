NDN-CPP: A Named Data Networking client library for C++ and C - Android
=======================================================================

This are instructions to cross-compile NDN-CPP for Android.

Prerequisites
=============

* Required: Android Studio (version 2.2 or higher)
* Required: Android LLDB, CMake and NDK
* Required: OpenSSL 1.0.x.
* Required: perl (for configuring the OpenSSL build)
* Optional: SQLite3 (compiled from the source in contrib/sqlite3)

## macOS 10.12

Required: Install Android Studio from https://developer.android.com/studio/index.html .
(Tested with Android Studio 2.3.3 .)

Required: In the Android Studio SDK Manager, install LLDB, CMake and NDK following the instructions at
https://developer.android.com/ndk/guides/index.html

Set the environment variables ANDROID_SDK_ROOT and ANDROID_NDK_ROOT to the installed location, for example:

    export ANDROID_SDK_ROOT=~/Library/Android/sdk
    export ANDROID_NDK_ROOT=~/Library/Android/sdk/ndk-bundle

Add the folders with ndk-build and adb to the PATH, for example:

    export PATH=$PATH:$ANDROID_NDK_ROOT:$ANDROID_SDK_ROOT/platform-tools

Required: Download the latest OpenSSL 1.0.x from https://www.openssl.org/source . Extract the files, for example:

    tar xvfz openssl-1.0.2l.tar.gz

Prepare OpenSSL
===============

(These instructions are taken from https://wiki.openssl.org/index.php/Android .)
Make sure the environment variables ANDROID_SDK_ROOT and ANDROID_NDK_ROOT are set (see above).
To run the OpenSSL setup script, in the following change <NDN-CPP-root> to the root of the NDN-CPP distribution. In
a terminal, enter:

    . <NDN-CPP-root>/android-native/setenv-android.sh

(This runs the script from https://wiki.openssl.org/images/7/70/Setenv-android.sh which is configured for
android-ndk-r9 and arm . You may need to edit it to change _ANDROID_NDK, _ANDROID_ARCH, _ANDROID_EABI
and _ANDROID_API . For details, see https://wiki.openssl.org/index.php/Android#Adjust_the_Cross-Compile_Script .)

In a terminal, change directory to the extracted openssl distribution and enter:

    perl -pi -e 's/install: all install_docs install_sw/install: install_docs install_sw/g' Makefile.org
    ./config shared no-asm no-ssl2 no-ssl3 no-comp no-hw no-engine --openssldir=.
    make depend

Build
=====

Create a new Android project with the following configuration:

* In the New Project wizard, name the project (e.g. "ndn-cpp-native"). Be sure to check "Include C++ support".
* Select a minimum SDK. This has been tested with "API 21: Android 5.0 (Lollipop)".
* Select the Basic Activity template.
* In the Customize C++ Support screen, for the C++ Standard, select "C++11". Check "Exceptions Support"
  and "Runtime Type Information Support".
* Click Finish to complete the New Project wizard.

In a terminal, change directory to the root of your Android Studio project,
for example "~/AndroidStudioProjects/ndn-cpp-native".

To make a link to NDN-CPP, in the following change <NDN-CPP-root> to the root of the NDN-CPP distribution:

    ln -s <NDN-CPP-root> app/src/ndn-cpp

To make a link to OpenSSL, in the following change <OpenSSL> to the extracted OpenSSL distribution:

    ln -s <OpenSSL> app/src/openssl

The Android config.h mainly selects the std shared_ptr and other defaults.
Copy it to the NDN-CPP include folder:

    cp app/src/ndn-cpp/android-native/ndn-cpp-config.h app/src/ndn-cpp/include/ndn-cpp

Copy the project mk files:

    cp app/src/ndn-cpp/android-native/Android.mk app
    cp app/src/ndn-cpp/android-native/Application.mk app

(This uses SQLite by default. To omit SQLite, edit ndn-cpp-config.h to remove the line
`#define NDN_CPP_HAVE_SQLITE3` and edit Android.mk to remove sqlite3 from the line
`LOCAL_SHARED_LIBRARIES := openssl sqlite3` . Also, you don't need to install
libsqlite3.so on your Android.)

Use NDK to build:

    ndk-build NDK_PROJECT_PATH=`pwd` NDK_LIBS_OUT=`pwd`/app/build/intermediates/cmake/debug/obj/armeabi APP_BUILD_SCRIPT=app/Android.mk NDK_APPLICATION_MK=app/Application.mk

The libndn-cpp.so library and other files are in a subfolder based on the APP_ABI target in Application.mk .
For example, if the target is armeabi-v7a then the files are in
app/build/intermediates/cmake/debug/obj/armeabi/armeabi-v7a . The default Android.mk
builds an Android executable ndn-example from test-encode-decode-data.

## Running on Android with adb shell

To copy the library files and example application to Android (attached by USB cable), enter:

    adb push app/build/intermediates/cmake/debug/obj/armeabi/armeabi-v7a/* /data/local/tmp

Start the adb shell:

    adb shell

In the adb shell, set the library path and run the application:

    cd /data/local/tmp
    export LD_LIBRARY_PATH=`pwd`
    ./ndn-example

## Unity on Android

To use libndn-cpp.so in your Unity on Android project, copy libndn-cpp.so to the
subfolder Assets/Plugins/Android of your Unity project folder. (Create this
folder if it doesn't exist.)
