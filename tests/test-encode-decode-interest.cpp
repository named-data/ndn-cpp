/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/encoding/tlv-wire-format.hpp>

using namespace std;
using namespace ndn;

uint8_t BinaryXmlInterest[] = {
0x01, 0xd2,
  0xf2, 0xfa, 0x9d, 0x6e, 0x64, 0x6e, 0x00, 0xfa, 0x9d, 0x61, 0x62, 0x63, 0x00, 0x00, 
  0x05, 0x9a, 0x9e, 0x31, 0x32, 0x33, 0x00, 
  0x05, 0xa2, 0x8e, 0x34, 0x00,
  0x03, 0xe2, 
    0x02, 0x85, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
  0x00,
  0x02, 0xda, 0xfa, 0x9d, 0x61, 0x62, 0x63, 0x00, 0xea, 0x00, 0x00, 
  0x05, 0xaa, 0x8e, 0x31, 0x00, 
  0x02, 0xfa, 0x8e, 0x34, 0x00, 
  0x02, 0xd2, 0x8e, 0x32, 0x00, 
  0x03, 0x82, 0x9d, 0x01, 0xe0, 0x00, 0x00, 
  0x02, 0xca, 0xb5, 0x61, 0x62, 0x61, 0x62, 0x61, 0x62, 0x00, 
0x00, 
1
};

uint8_t TlvInterest[] = {
0x05, 0x53, // Interest
  0x07, 0x0A, 0x08, 0x03, 0x6E, 0x64, 0x6E, 0x08, 0x03, 0x61, 0x62, 0x63, // Name
  0x09, 0x38, // Selectors
    0x0D, 0x01, 0x04, // MinSuffixComponents
    0x0E, 0x01, 0x06, // MaxSuffixComponents
    0x1C, 0x22, // KeyLocator
      0x1D, 0x20, // KeyLocatorDigest
                  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x10, 0x07, // Exclude
      0x08, 0x03, 0x61, 0x62, 0x63, // NameComponent
      0x13, 0x00, // Any
    0x11, 0x01, 0x01, // ChildSelector
    0x12, 0x00, // MustBeFesh
  0x0A, 0x04, 0x61, 0x62, 0x61, 0x62,	// Nonce
  0x0B, 0x01, 0x02, // Scope
  0x0C, 0x02, 0x75, 0x30, // InterestLifetime
1
};

static void dumpInterest(const Interest& interest)
{
  cout << "name: " << interest.getName().toUri() << endl;
  cout << "minSuffixComponents: ";
  if (interest.getMinSuffixComponents() >= 0)
    cout << interest.getMinSuffixComponents() << endl;
  else
    cout << "<none>" << endl;
  cout << "maxSuffixComponents: ";
  if (interest.getMaxSuffixComponents() >= 0)
    cout << interest.getMaxSuffixComponents() << endl;
  else
    cout << "<none>" << endl;
  cout << "keyLocator: ";
  if ((int)interest.getKeyLocator().getType() >= 0) {
    if (interest.getKeyLocator().getType() == ndn_KeyLocatorType_KEY)
      cout << "Key: " << interest.getKeyLocator().getKeyData().toHex() << endl;
    else if (interest.getKeyLocator().getType() == ndn_KeyLocatorType_CERTIFICATE)
      cout << "Certificate: " << interest.getKeyLocator().getKeyData().toHex() << endl;
    else if (interest.getKeyLocator().getType() == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST)
      cout << "KeyLocatorDigest: " << interest.getKeyLocator().getKeyData().toHex() << endl;
    else if (interest.getKeyLocator().getType() == ndn_KeyLocatorType_KEYNAME)
      cout << "KeyName: " << interest.getKeyLocator().getKeyName().toUri() << endl;
    else
      cout << "<unrecognized ndn_KeyLocatorType " << interest.getKeyLocator().getType() << ">" << endl;
  }
  else
    cout << "<none>" << endl;

  cout << "exclude: " 
       << (interest.getExclude().size() > 0 ? interest.getExclude().toUri() : "<none>") << endl;
  cout << "lifetimeMilliseconds: ";
  if (interest.getInterestLifetimeMilliseconds() >= 0)
    cout << interest.getInterestLifetimeMilliseconds() << endl;
  else
    cout << "<none>" << endl;
  cout << "childSelector: ";
  if (interest.getChildSelector() >= 0)
    cout << interest.getChildSelector() << endl;
  else
    cout << "<none>" << endl;
  cout << "mustBeFresh: " << (interest.getMustBeFresh() ? "true" : "false") << endl;
  cout << "scope: ";
  if (interest.getScope() >= 0)
    cout << interest.getScope() << endl;
  else
    cout << "<none>" << endl;
  cout << "nonce: " 
       << (interest.getNonce().size() > 0 ? interest.getNonce().toHex() : "<none>") << endl;
}

int main(int argc, char** argv)
{
  try {
    Interest interest;
    // Note: While we transition to the TLV wire format, check if it has been made the default.
    if (WireFormat::getDefaultWireFormat() == TlvWireFormat::get())
      interest.wireDecode(TlvInterest, sizeof(TlvInterest));
    else
      interest.wireDecode(BinaryXmlInterest, sizeof(BinaryXmlInterest));
    cout << "Interest:" << endl;
    dumpInterest(interest);
    
    Blob encoding = interest.wireEncode();
    cout << endl << "Re-encoded interest " << encoding.toHex() << endl;

    Interest reDecodedInterest;
    reDecodedInterest.wireDecode(*encoding);
    cout << "Re-decoded Interest:" << endl;
    dumpInterest(reDecodedInterest);
    
    Interest freshInterest(Name("/ndn/abc"));
    freshInterest.setMinSuffixComponents(4);
    freshInterest.setMaxSuffixComponents(6);
    freshInterest.getKeyLocator().setType(ndn_KeyLocatorType_KEY_LOCATOR_DIGEST);
    uint8_t digest[] = {
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    freshInterest.getKeyLocator().setKeyData(Blob(digest, sizeof(digest)));
    freshInterest.getExclude().appendComponent(Name("abc")[0]).appendAny();
    freshInterest.setInterestLifetimeMilliseconds(30000);
    freshInterest.setChildSelector(1);
    freshInterest.setMustBeFresh(true);
    freshInterest.setScope(2);

    Interest reDecodedFreshInterest; 
    reDecodedFreshInterest.wireDecode(*freshInterest.wireEncode());    
    cout << endl << "Re-decoded fresh Interest:" << endl;
    dumpInterest(reDecodedFreshInterest);
  } catch (exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}
