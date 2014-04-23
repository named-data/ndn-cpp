/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <stdexcept>
#include <algorithm>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "base64.hpp"

using namespace std;

namespace ndn {

string
toBase64(const uint8_t* array, size_t arrayLength, bool addNewlines)
{
  BIO *base64 = BIO_new(BIO_f_base64());
  if (!base64)
    throw runtime_error("toBase64: BIO_new failed");
  
  if (!addNewlines)
    BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);

  BIO *outputBuffer = BIO_new(BIO_s_mem());
  if (!outputBuffer) {
    BIO_free(base64);
    throw runtime_error("toBase64: BIO_new failed");
  }
  outputBuffer = BIO_push(base64, outputBuffer);

  if (BIO_write(base64, array, arrayLength) <= 0) {
    BIO_free_all(outputBuffer);
    throw runtime_error("toBase64: BIO_write failed");
  }

  BIO_flush(base64);
  char *bufferPointer;
  size_t bufferSize = BIO_get_mem_data(outputBuffer, &bufferPointer);
  
  string result(bufferPointer, bufferSize);
  BIO_free_all(outputBuffer);
  return result;
}

void
fromBase64(const string& input, vector<uint8_t>& output)
{
  // openssl doesn't like whitespace, so remove it.
  string cleanInput(input);
  cleanInput.erase
    (remove_if(cleanInput.begin(), cleanInput.end(), ::isspace), 
     cleanInput.end());
  
  BIO *base64 = BIO_new(BIO_f_base64());
  if (!base64)
    throw runtime_error("fromBase64: BIO_new failed");
  BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);
  
  BIO *inputBuffer = BIO_new_mem_buf((char*)&cleanInput[0], cleanInput.size());
  if (!inputBuffer) {
    BIO_free(base64);
    throw runtime_error("fromBase64: BIO_new_mem_buf failed");
  }
  inputBuffer = BIO_push(base64, inputBuffer);
  
  // The output will be shorter than the input.
  output.resize(cleanInput.size());
  int outputLength = BIO_read(inputBuffer, &output[0], cleanInput.size());
  if (outputLength < 0) {
    BIO_free_all(inputBuffer);  
    throw runtime_error("fromBase64: BIO_read failed to decode");    
  }
  
  output.resize(outputLength);
  BIO_free_all(inputBuffer);  
}

}
