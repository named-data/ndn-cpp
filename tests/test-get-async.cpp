/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/face.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;
#if NDN_CPP_HAVE_STD_FUNCTION
// In the std library, the placeholders are in a different namespace than boost.
using namespace func_lib::placeholders;
#endif

class Counter
{
public:
  Counter() {
    callbackCount_ = 0;
  }
  
  void onData(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data)
  {
    ++callbackCount_;
    cout << "Got data packet with name " << data->getName().toUri() << endl;
    for (size_t i = 0; i < data->getContent().size(); ++i)
      cout << (*data->getContent())[i];
    cout << endl;  
  }

  void onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
  {
    ++callbackCount_;
    cout << "Time out for interest " << interest->getName().toUri() << endl;    
  }
  
  int callbackCount_;
};

int main(int argc, char** argv)
{
  try {
    Face face("aleph.ndn.ucla.edu");
    
    // Counter holds data used by the callbacks.
    Counter counter;
    
    Name name1("/ndn/edu/ucla/remap/ndn-js-test/howdy.txt/%FD%052%A1%DF%5E%A4");    
    cout << "Express name " << name1.toUri() << endl;
    // Use bind to pass the counter object to the callbacks.
    face.expressInterest(name1, bind(&Counter::onData, &counter, _1, _2), bind(&Counter::onTimeout, &counter, _1));
    
    // Try to get anything.
    Name name2("/");
    cout << "Express name " << name2.toUri() << endl;
    face.expressInterest(name2, bind(&Counter::onData, &counter, _1, _2), bind(&Counter::onTimeout, &counter, _1));
    
    // Expect this to time out.
    Name name3("/test/timeout");
    cout << "Express name " << name3.toUri() << endl;
    face.expressInterest(name3, bind(&Counter::onData, &counter, _1, _2), bind(&Counter::onTimeout, &counter, _1));

    // The main event loop.
    while (counter.callbackCount_ < 3) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
