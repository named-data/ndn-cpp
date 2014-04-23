/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <sstream>
#include <stdexcept>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <ndn-cpp/common.hpp>

using namespace std;

namespace ndn {

string 
toHex(const uint8_t* array, size_t arrayLength) 
{
  if (arrayLength == 0)
    return "";
  
  ostringstream result;
  result.flags(ios::hex);
  for (size_t i = 0; i < arrayLength; ++i) {
    uint8_t x = array[i];
    if (x < 16)
      result << '0';
    result << (unsigned int)x;
  }

  return result.str();
}

string
toBase64(const uint8_t* array, size_t arrayLength)
{
  BIO *base64 = BIO_new(BIO_f_base64());
  if (!base64)
    throw runtime_error("toBase64: BIO_new failed");
  
  BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);

  BIO *buffer = BIO_new(BIO_s_mem());
  if (!buffer)
    throw runtime_error("toBase64: BIO_new failed");
  BIO_push(base64, buffer);

  if (BIO_write(base64, array, arrayLength) <= 0)
    throw runtime_error("toBase64: BIO_write failed");

  BIO_flush(base64);
  char *bufferPointer;
  size_t bufferSize = BIO_get_mem_data(buffer, &bufferPointer);
  
  string result(bufferPointer, bufferSize);
  BIO_free(buffer);
  BIO_free(base64);
  return result;
}

}
