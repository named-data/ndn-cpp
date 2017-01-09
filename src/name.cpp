/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#include <stdexcept>
#include <algorithm>
#include <string.h>
#include <stdexcept>
#include <ndn-cpp/name.hpp>
#include "c/util/ndn_memory.h"
#include "encoding/tlv-encoder.hpp"

using namespace std;

namespace ndn {

/**
 * Convert the hex character to an integer from 0 to 15, or -1 if not a hex character.
 * @param c
 * @return
 */
static int
fromHexChar(uint8_t c)
{
  if (c >= '0' && c <= '9')
    return (int)c - (int)'0';
  else if (c >= 'A' && c <= 'F')
    return (int)c - (int)'A' + 10;
  else if (c >= 'a' && c <= 'f')
    return (int)c - (int)'a' + 10;
  else
    return -1;
}

/**
 * Make a Blob value by decoding the hexString between beginOffset and
 * endOffset.
 * @param hexString The hex string.
 * @param beginOffset The offset in hexString of the beginning of the
 * portion to decode.
 * @param endOffset The offset in hexString of the end of the portion to
 * decode.
 * @return The Blob value. If the hexString is not a valid hex string, then
 * the Blob has a null pointer.
 */
static Blob
fromHex(const string& hexString, size_t beginOffset, size_t endOffset)
{
  ostringstream result;

  for (size_t i = beginOffset; i < endOffset; ++i) {
    if (hexString[i] == ' ')
      // Skip whitespace.
      continue;
    if (i + 1 >= endOffset)
      // Only one hex digit. Ignore.
      break;

    int hi = fromHexChar(hexString[i]);
    int lo = fromHexChar(hexString[i + 1]);

    if (hi < 0 || lo < 0)
      // Invalid hex characters.
      return Blob();

    result << (uint8_t)(16 * hi + lo);

    // Skip past the second digit.
    i += 1;
  }

  string resultString = result.str();
  return Blob((uint8_t*)&resultString[0], resultString.size());
}

/**
 * Return a copy of str, converting each escaped "%XX" to the char value.
 * @param str
 */
static string
unescape(const string& str)
{
  ostringstream result;

  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '%' && i + 2 < str.size()) {
      int hi = fromHexChar(str[i + 1]);
      int lo = fromHexChar(str[i + 2]);

      if (hi < 0 || lo < 0)
        // Invalid hex characters, so just keep the escaped string.
        result << str[i] << str[i + 1] << str[i + 2];
      else
        result << (uint8_t)(16 * hi + lo);

      // Skip ahead past the escaped value.
      i += 2;
    }
    else
      // Just copy through.
      result << str[i];
  }

  return result.str();
}

const uint8_t Name::Component::FINAL_SEGMENT_PREFIX[] = { 0xC1, '.', 'M', '.', 'F', 'I', 'N', 'A', 'L', 0x00 };
size_t Name::Component::FINAL_SEGMENT_PREFIX_LENGTH = sizeof(Name::Component::FINAL_SEGMENT_PREFIX);

uint64_t
Name::Component::toNumberWithMarker(uint8_t marker) const
{
  NameLite::Component componentLite;
  get(componentLite);
  uint64_t result;

  ndn_Error error;
  if ((error = componentLite.toNumberWithMarker(marker, result)))
    throw runtime_error(ndn_getErrorString(error));

  return result;
}

uint64_t
Name::Component::toNumberWithPrefix(const uint8_t* prefix, size_t prefixLength) const
{
  NameLite::Component componentLite;
  get(componentLite);
  uint64_t result;

  ndn_Error error;
  if ((error = componentLite.toNumberWithPrefix(prefix, prefixLength, result)))
    throw runtime_error(ndn_getErrorString(error));

  return result;
}

bool
Name::Component::hasPrefix(const uint8_t* prefix, size_t prefixLength) const
{
  NameLite::Component componentLite;
  get(componentLite);
  return componentLite.hasPrefix(prefix, prefixLength) != 0;
}

Name::Component
Name::Component::fromNumber(uint64_t number)
{
  TlvEncoder encoder(8);
  encoder.writeNonNegativeInteger(number);
  return Name::Component(Blob(encoder.finish()));
}

Name::Component
Name::Component::fromNumberWithMarker(uint64_t number, uint8_t marker)
{
  TlvEncoder encoder(9);
  // Add the leading marker.
  encoder.writeNonNegativeInteger(marker & 0xff);
  encoder.writeNonNegativeInteger(number);
  return Name::Component(encoder.finish());
}

Name::Component
Name::Component::fromNumberWithPrefix(uint64_t number, const uint8_t* prefix, size_t prefixLength)
{
  // Initialize with the prefix
  ptr_lib::shared_ptr<vector<uint8_t> > value(new vector<uint8_t>(prefix, prefix + prefixLength));

  // First encode in little endian.
  while (number != 0) {
    value->push_back(number & 0xff);
    number >>= 8;
  }

  // Make it big endian.
  reverse(value->begin() + prefixLength, value->end());
  return Name::Component(Blob(value, false));
}

void
Name::Component::get(NameLite::Component& componentLite) const
{
  if (type_ == ndn_NameComponentType_IMPLICIT_SHA256_DIGEST)
    componentLite.setImplicitSha256Digest(value_);
  else
    componentLite = NameLite::Component(value_);
}

void
Name::Component::toEscapedString(std::ostringstream& result) const
{
  if (type_ == ndn_NameComponentType_IMPLICIT_SHA256_DIGEST) {
    result << "sha256digest=";
    value_.toHex(result);
  }
  else
    Name::toEscapedString(*value_, result);
}

std::string
Name::Component::toEscapedString() const
{
  ostringstream result;
  toEscapedString(result);
  return result.str();
}

uint64_t
Name::Component::toNumber() const
{
  NameLite::Component componentLite;
  get(componentLite);
  return componentLite.toNumber();
}

int
Name::Component::compare(const Name::Component& other) const
{
  // Imitate ndn_NameComponent_compare.
  if (type_ < other.type_)
    return -1;
  if (type_ > other.type_)
    return 1;

  if (value_.size() < other.value_.size())
    return -1;
  if (value_.size() > other.value_.size())
    return 1;

  // The components are equal length.  Just do a byte compare.
  return ndn_memcmp(value_.buf(), other.value_.buf(), value_.size());
}

Name::Component
Name::Component::fromImplicitSha256Digest(const Blob& digest)
{
  if (digest.size() != ndn_SHA256_DIGEST_SIZE)
    throw runtime_error
      ("Name::Component::fromImplicitSha256Digest: The digest length must be 32 bytes");

  Component result(digest);
  result.type_ = ndn_NameComponentType_IMPLICIT_SHA256_DIGEST;
  return result;
}

Name::Component
Name::Component::getSuccessor() const
{
  // Allocate an extra byte in case the result is larger.
  ptr_lib::shared_ptr<vector<uint8_t> > result
    (new vector<uint8_t>(value_.size() + 1));

  bool carry = true;
  for (int i = (int)value_.size() - 1; i >= 0; --i) {
    if (carry) {
      (*result)[i] = value_.buf()[i] + 1;
      carry = ((*result)[i] == 0);
    }
    else
      (*result)[i] = value_.buf()[i];
  }

  if (carry)
    // Assume all the bytes were set to zero (or the component was empty).
    // In NDN ordering, carry does not mean to prepend a 1, but to make a
    // component one byte longer of all zeros.
    (*result)[result->size() - 1] = 0;
  else
    // We didn't need the extra byte.
    result->resize(value_.size());

  return Component(Blob(result, false));
}

void
Name::set(const char *uri_cstr)
{
  clear();

  string uri = uri_cstr;
  ndn_trim(uri);
  if (uri.size() == 0)
    return;

  size_t iColon = uri.find(':');
  if (iColon != string::npos) {
    // Make sure the colon came before a '/'.
    size_t iFirstSlash = uri.find('/');
    if (iFirstSlash == string::npos || iColon < iFirstSlash) {
      // Omit the leading protocol such as ndn:
      uri.erase(0, iColon + 1);
      ndn_trim(uri);
    }
  }

  // Trim the leading slash and possibly the authority.
  if (uri[0] == '/') {
    if (uri.size() >= 2 && uri[1] == '/') {
      // Strip the authority following "//".
      size_t iAfterAuthority = uri.find('/', 2);
      if (iAfterAuthority == string::npos)
        // Unusual case: there was only an authority.
        return;
      else {
        uri.erase(0, iAfterAuthority + 1);
        ndn_trim(uri);
      }
    }
    else {
      uri.erase(0, 1);
      ndn_trim(uri);
    }
  }

  size_t iComponentStart = 0;

  // Unescape the components.
  string sha256digestPrefix("sha256digest=");
  while (iComponentStart < uri.size()) {
    size_t iComponentEnd = uri.find("/", iComponentStart);
    if (iComponentEnd == string::npos)
      iComponentEnd = uri.size();

    Component component;
    if (iComponentStart + sha256digestPrefix.size() <= uri.size() &&
        uri.compare
          (iComponentStart, sha256digestPrefix.size(), sha256digestPrefix) == 0)
      component = Component::fromImplicitSha256Digest
            (fromHex(uri, iComponentStart + sha256digestPrefix.size(),
                     iComponentEnd));
    else
      component = Component
        (fromEscapedString(&uri[0], iComponentStart, iComponentEnd));

    // Ignore illegal components.  This also gets rid of a trailing '/'.
    if (component.getValue())
      append(component);

    iComponentStart = iComponentEnd + 1;
  }
}

void
Name::get(NameLite& nameLite) const
{
  nameLite.clear();
  for (size_t i = 0; i < components_.size(); ++i) {
    ndn_Error error;
    NameLite::Component component;
    components_[i].get(component);
    if ((error = nameLite.append(component)))
      throw runtime_error(ndn_getErrorString(error));
  }
}

void
Name::set(const NameLite& nameLite)
{
  clear();
  for (size_t i = 0; i < nameLite.size(); ++i)
    append(Component(nameLite.get(i)));
}

Name&
Name::append(const Name& name)
{
  if (&name == this)
    // Copying from this name, so need to make a copy first.
    return append(Name(name));

  for (size_t i = 0; i < name.components_.size(); ++i)
    append(name.components_[i]);

  return *this;
}

string
Name::toUri(bool includeScheme) const
{
  if (components_.size() == 0)
    return includeScheme ? "ndn:/" : "/";

  ostringstream result;
  if (includeScheme)
    result << "ndn:";
  for (size_t i = 0; i < components_.size(); ++i) {
    result << "/";
    components_[i].toEscapedString(result);
  }

  return result.str();
}

Name
Name::getSubName(int iStartComponent, size_t nComponents) const
{
  if (iStartComponent < 0)
    iStartComponent = components_.size() - (-iStartComponent);

  Name result;

  size_t iEnd = min(iStartComponent + nComponents, components_.size());
  for (size_t i = iStartComponent; i < iEnd; ++i)
    result.components_.push_back(components_[i]);

  return result;
}

bool
Name::equals(const Name& name) const
{
  if (components_.size() != name.components_.size())
    return false;

  // Check from last to first since the last components are more likely to differ.
  for (int i = components_.size() - 1; i >= 0; --i) {
    if (*components_[i].getValue() != *name.components_[i].getValue())
      return false;
  }

  return true;
}

Name
Name::getSuccessor() const
{
  if (size() == 0) {
    // Return "/%00".
    Name result;
    result.append((const uint8_t*)"\0", 1);
    return result;
  }
  else
    return getPrefix(-1).append(get(-1).getSuccessor());
}

bool
Name::match(const Name& name) const
{
  // Imitate ndn_Name_match.

  // This name is longer than the name we are checking it against.
  if (components_.size() > name.components_.size())
    return false;

  // Check if at least one of given components doesn't match. Check from last to
  // first since the last components are more likely to differ.
  for (int i = components_.size() - 1; i >= 0; --i) {
    if (*components_[i].getValue() != *name.components_[i].getValue())
      return false;
  }

  return true;
}

Blob
Name::fromEscapedString(const char *escapedString, size_t beginOffset, size_t endOffset)
{
  string trimmedString(escapedString + beginOffset, escapedString + endOffset);
  ndn_trim(trimmedString);
  string value = unescape(trimmedString);

  if (value.find_first_not_of(".") == string::npos) {
    // Special case for component of only periods.
    if (value.size() <= 2)
      // Zero, one or two periods is illegal.  Ignore this component.
      return Blob();
    else
      // Remove 3 periods.
      return Blob((const uint8_t *)&value[3], value.size() - 3);
  }
  else
    return Blob((const uint8_t *)&value[0], value.size());
}

Blob
Name::fromEscapedString(const char *escapedString)
{
  return fromEscapedString(escapedString, 0, ::strlen(escapedString));
}

void
Name::toEscapedString(const vector<uint8_t>& value, ostringstream& result)
{
  bool gotNonDot = false;
  for (unsigned i = 0; i < value.size(); ++i) {
    if (value[i] != 0x2e) {
      gotNonDot = true;
      break;
    }
  }
  if (!gotNonDot) {
    // Special case for component of zero or more periods.  Add 3 periods.
    result << "...";
    for (size_t i = 0; i < value.size(); ++i)
      result << '.';
  }
  else {
    // In case we need to escape, set to upper case hex and save the previous flags.
    ios::fmtflags saveFlags = result.flags(ios::hex | ios::uppercase);

    for (size_t i = 0; i < value.size(); ++i) {
      uint8_t x = value[i];
      // Check for 0-9, A-Z, a-z, (+), (-), (.), (_)
      if ((x >= 0x30 && x <= 0x39) || (x >= 0x41 && x <= 0x5a) ||
          (x >= 0x61 && x <= 0x7a) || x == 0x2b || x == 0x2d ||
          x == 0x2e || x == 0x5f)
        result << x;
      else {
        result << '%';
        if (x < 16)
          result << '0';
        result << (unsigned int)x;
      }
    }

    // Restore.
    result.flags(saveFlags);
  }
}

string
Name::toEscapedString(const vector<uint8_t>& value)
{
  ostringstream result;
  toEscapedString(value, result);
  return result.str();
}

const Name::Component&
Name::get(int i) const
{
  if (i >= 0) {
    if (i >= (int)components_.size())
      throw runtime_error("Name.get: Index is out of bounds");

    return components_[i];
  }
  else {
    // Negative index.
    if (i < -((int)components_.size()))
      throw runtime_error("Name.get: Index is out of bounds");

    return components_[components_.size() - (-i)];
  }
}

int
Name::compare
  (int iStartComponent, size_t nComponents, const Name& other,
   int iOtherStartComponent, size_t nOtherComponents) const
{
  if (iStartComponent < 0)
    iStartComponent = components_.size() - (-iStartComponent);
  if (iStartComponent < 0)
    iOtherStartComponent = other.components_.size() - (-iOtherStartComponent);

  nComponents = min(nComponents, size() - iStartComponent);
  nOtherComponents = min(nOtherComponents, other.size() - iOtherStartComponent);

  size_t count = min(nComponents, nOtherComponents);
  for (size_t i = 0; i < count; ++i) {
    int comparison = components_[iStartComponent + i].compare
      (other.components_[iOtherStartComponent + i]);
    if (comparison == 0)
      // The components at this index are equal, so check the next components.
      continue;

    // Otherwise, the result is based on the components at this index.
    return comparison;
  }

  // The components up to min(this.size(), other.size()) are equal, so the
  // shorter name is less.
  if (nComponents < nOtherComponents)
    return -1;
  else if (nComponents > nOtherComponents)
    return 1;
  else
    return 0;
}

}
