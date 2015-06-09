NDN-CPP Lite: A light-weight C++ layer over the C language core in NDN-CPP
--------------------------------------------------------------------------

NDN-CPP is a Named Data Networking client library for C++ and C. The main C++
API uses the Standard Library classes like std::vector and shared_ptr to
automatically manage memory in objects. For data packet encoding, decoding and
network transport, the C++ API calls an inner core written in pure C which does
not make any assumptions about memory management or support libraries.

Some low-power platforms like an Arduino microcontroller don't support the C++
Standard Library or run-time info for exceptions. To support such platforms, the
NDN-CPP Lite API was developed which, like the C core, does not make assumptions
about memory management or support libraries. While functionally equivalent to
the C core, the NDN-CPP Lite takes advantage of C++ syntax to simplify the API.
For example, the following C code initializes a MetaInfo struct:

    struct ndn_MetaInfo metaInfo;
    ndn_MetaInfo_initialize(&metaInfo);

The following equivalent NDN-CPP Lite code initializes a MetaInfoLite object:

    MetaInfoLite metaInfo;

The MetaInfoLite constructor internally calls the same ndn_MetaInfo_initialize
function, but C++ syntax, method overloading and namespace support makes the
NDN-CPP Lite code cleaner and less error-prone.

NDN-CPP Lite itself does not use "new", std::vector, shared_ptr or other
memory manipulation functions. The application is responsible for managing
memory and providing pointers to NDN-CPP Lite. The following code creates the
name "/ndn/ucla":

    ndn_NameComponent nameComponents[10];
    NameLite name(nameComponents, 10);
    ndn_Error error;
    if ((error = name.append("ndn")))
      return error;
    if ((error = name.append("ucla")))
      return error;

The code cannot allocate new memory to enlarge the name components array, so a
sufficiently large array is provided to the NameLite constructor. The append
method is provided a pre-allocated buffer for the component value, in this case
a static C string. The append method returns an error if there is not enough
room in the nameComponents array to add another entry to point to the new
component value. (NDN-CPP Lite uses error codes instead of exceptions.)

The file examples/arduino/analog-reading/analog-reading.ino uses NDN-CPP Lite
for an Arduino application which registers a prefix, receives an interest over
TCP and returns a signed (with HMAC) data packet holding a measurement value.
The compiled application is about 28 kilobytes as required by the small
microcontroller program space.

See the inline documentation for the classes and methods in include/ndn-cpp/lite.

