#define STR1(x) #x
#define STR2(x) STR1(x)
#define CONCAT(x, y) x ## y

// This makes a string for #include from the NDN-CPP root directory.
// It should include the absolute path up to ndn-cpp, except put "ndn-c"
// instead of "ndn-cpp". Use it like this:
// #include NDN_CPP_ROOT(pp/c/name.c)
// We split "ndn-cpp" into "ndn-c" and "pp" because the Arduino compiler won't
// accept NDN_CPP_SRC(/src/c/name.c) with a starting slash.
// We have to use an absolute path because the Arduino compiler won't
// include a relative file.
#define NDN_CPP_ROOT(x) STR2(CONCAT(/please/fix/NDN_CPP_ROOT/in/ndn_cpp_root.h/ndn-c, x))

