/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <ndn-cpp/encoding/tlv-wire-format.hpp>

namespace ndn {

// NDN_CPP_USE_DEFAULT_TLV takes precedence over NDN_CPP_USE_DEFAULT_BINARY_XML.
#if (defined(NDN_CPP_USE_DEFAULT_TLV) || !defined(NDN_CPP_USE_DEFAULT_BINARY_XML))
// This is declared in the WireFormat class.
WireFormat*
WireFormat::newInitialDefaultWireFormat() 
{
  return TlvWireFormat::get();
}
#endif
  
TlvWireFormat* TlvWireFormat::instance_ = 0;

}
