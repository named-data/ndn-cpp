/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_BASE64_HPP
#define NDN_BASE64_HPP

#include <string>
#include <vector>
#include <ndn-cpp/common.hpp>

namespace ndn {

/**
 * Return the base64 representation of the bytes in array.
 * @param array The array of bytes.
 * @param arrayLength The number of bytes in array.
 * @param addNewlines (optional) If true, add newlines to the output (good for 
 * writing to a file).  If omitted, do not add newlines.
 * @return The base64 string.
 */
std::string
toBase64(const uint8_t* array, size_t arrayLength, bool addNewlines = false);

/**
 * Return the base64 representation of the bytes in array.
 * @param array The array of bytes.
 * @param addNewlines (optional) If true, add newlines to the output (good for 
 * writing to a file).  If omitted, do not add newlines.
 * @return The base64 string.
 */
static inline std::string 
toBase64(const std::vector<uint8_t>& array, bool addNewlines = false)
{
  return toBase64(&array[0], array.size(), addNewlines);
}

/**
 * Decode the base64 input string and write the binary result to output.
 * @param input The input base64 string.
 * @param output Write the result to output starting at index 0, calling
 * output.resize() as needed.
 */
void
fromBase64(const std::string& input, std::vector<uint8_t>& output);


}

#endif
