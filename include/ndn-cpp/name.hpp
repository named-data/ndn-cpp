/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 * @author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_NAME_HPP
#define NDN_NAME_HPP

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include "util/blob.hpp"

struct ndn_NameComponent;
struct ndn_Name;

namespace ndn {
    
/**
 * A Name holds an array of Name::Component and represents an NDN name.
 */
class Name {
public:
  /**
   * A Name::Component holds a read-only name component value.
   */
  class Component {
  public:
    /**
     * Create a new Name::Component with a null value.
     */
    Component()
    {    
    }
  
    /**
     * Create a new Name::Component, copying the given value.
     * @param value The value byte array.
     */
    Component(const std::vector<uint8_t>& value) 
    : value_(value)
    {
    }

    /**
     * Create a new Name::Component, copying the given value.
     * @param value Pointer to the value byte array.
     * @param valueLen Length of value.
     */
    Component(const uint8_t *value, size_t valueLen) 
    : value_(value, valueLen)
    {
    }
    
    /**
     * Create a new Name::Component, taking another pointer to the Blob value.
     * @param value A blob with a pointer to an immutable array.  The pointer is copied.
     */
    Component(const Blob &value)
    : value_(value)
    {
    }
  
    /**
     * Set the componentStruct to point to this component, without copying any memory.
     * WARNING: The resulting pointer in componentStruct is invalid after a further use of this object which could reallocate memory.
     * @param componentStruct The C ndn_NameComponent struct to receive the pointer.
     */
    void 
    get(struct ndn_NameComponent& componentStruct) const;
  
    const Blob& 
    getValue() const { return value_; }

    /**
     * Write this component value to result, escaping characters according to the NDN URI Scheme.
     * This also adds "..." to a value with zero or more ".".
     * @param result the string stream to write to.
     */
    void 
    toEscapedString(std::ostringstream& result) const
    {
      Name::toEscapedString(*value_, result);
    }

    /**
     * Convert this component value by escaping characters according to the NDN URI Scheme.
     * This also adds "..." to a value with zero or more ".".
     * @return The escaped string.
     */
    std::string
    toEscapedString() const
    {
      return Name::toEscapedString(*value_);
    }
    
    /**
     * Interpret this name component as a network-ordered number and return an integer.
     * @return The integer number.
     */
    uint64_t
    toNumber() const;

    /**
     * Interpret this name component as a network-ordered number with a marker and return an integer.
     * @param marker The required first byte of the component.
     * @return The integer number.
     * @throw runtime_error If the first byte of the component does not equal the marker.
     */
    uint64_t
    toNumberWithMarker(uint8_t marker) const;

    /**
     * Interpret this name component as a network-ordered number with a prefix and return an integer.
     * @param prefix The required prefix of the component.
     * @param prefixLength The length of prefix.
     * @return The integer number.
     * @throw runtime_error If the first bytes of the component do not equal the prefix.
     */
    uint64_t
    toNumberWithPrefix(const uint8_t* prefix, size_t prefixLength) const;
    
    /**
     * Check if this name component begins with the given prefix.
     * @param prefix The required prefix of the component.
     * @param prefixLength The length of prefix.
     * @return true if this component begins with the prefix.
     */
    bool
    hasPrefix(const uint8_t* prefix, size_t prefixLength) const;

    /**
     * Interpret this name component as a segment number according to NDN name conventions (a network-ordered number 
     * where the first byte is the marker 0x00).
     * @return The integer segment number.
     * @throw runtime_error If the first byte of the component is not the expected marker.
     */
    uint64_t
    toSegment() const
    {
      return toNumberWithMarker(0x00);
    }
    
    /**
     * @deprecated Use toSegment.
     */
    uint64_t
    toSeqNum() const
    {
      return toSegment();
    }

    /**
     * Check if this name component is a final segment number so that toFinalSegment would be able to return the value.
     * @return true if this is a final segment number.
     */
    bool
    isFinalSegment() const { return hasPrefix(getFinalSegmentPrefix(), getFinalSegmentPrefixLength()); }
    
    /**
     * Interpret this name component as a final segment number which has the bytes of getFinalSegmentPrefix() followed
     * by a standard segment number as would be returned by toSegment.
     * @return The integer final segment number.
     * @throw runtime_error If the first bytes of the component is not the expected prefix for a final segment.
     */
    uint64_t
    toFinalSegment() const
    {
      return toNumberWithPrefix(getFinalSegmentPrefix(), getFinalSegmentPrefixLength());
    }

    /**
     * Interpret this name component as a version number according to NDN name conventions (a network-ordered number 
     * where the first byte is the marker 0xFD).  Note that this returns the exact number from the component
     * without converting it to a time representation.
     * @return The integer segment number.
     * @throw runtime_error If the first byte of the component is not the expected marker.
     */
    uint64_t
    toVersion() const
    {
      return toNumberWithMarker(0xFD);
    }
        
    /**
     * Create a component whose value is the network-ordered encoding of the number.
     * Note: if the number is zero, the result is empty.
     * @param number The number to be encoded.
     * @return The component value.
     */
    static Component 
    fromNumber(uint64_t number);
    
    /**
     * Create a component whose value is the marker appended with the network-ordered encoding of the number.
     * Note: if the number is zero, no bytes are used for the number - the result will have only the marker.
     * @param number The number to be encoded.  
     * @param marker The marker to use as the first byte of the component.
     * @return The component value.
     */
    static Component 
    fromNumberWithMarker(uint64_t number, uint8_t marker);
    
    /**
     * Create a component whose value is the prefix appended with the network-ordered encoding of the number.
     * Note: if the number is zero, no bytes are used for the number - the result will have only the prefix.
     * @param number The number to be encoded.  
     * @param prefix The prefix to use as the first bytes of the component.
     * @param prefixLength The length of prefix.
     * @return The component value.
     */
    static Component 
    fromNumberWithPrefix(uint64_t number, const uint8_t* prefix, size_t prefixLength);

    /**
     * Get the FINAL_SEGMENT_PREFIX which has the bytes of the component prefix used by toFinalSegment, etc.
     * @return A pointer to the buffer. 
     */
    static const uint8_t*getFinalSegmentPrefix() { return FINAL_SEGMENT_PREFIX; }
    
    /**
     * Get the length of getFinalSegmentPrefix().
     * @return The length of getFinalSegmentPrefix().
     */
    static size_t getFinalSegmentPrefixLength() { return FINAL_SEGMENT_PREFIX_LENGTH; }
    
    /**
     * Check if this is the same component as other.
     * @param other The other Component to compare with.
     * @return true if the components are equal, otherwise false.
     */
    bool
    equals(const Component& other) const
    {
      return *value_ == *other.value_;
    }
    
    /**
     * Check if this is the same component as other.
     * @param other The other Component to compare with.
     * @return true if the components are equal, otherwise false.
     */
    bool
    operator == (const Component& other) const { return equals(other); }

    /**
     * Check if this is not the same component as other.
     * @param other The other Component to compare with.
     * @return true if the components are not equal, otherwise false.
     */
    bool
    operator != (const Component& other) const { return !equals(other); }
    
    /**
     * Compare this to the other Component using NDN canonical ordering.
     * @param other The other Component to compare with.
     * @return 0 If they compare equal, -1 if *this comes before other in the canonical ordering, or
     * 1 if *this comes after other in the canonical ordering.
     *
     * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
     */
    int
    compare(const Component& other) const;
  
    /**
     * Return true if this is less than or equal to the other Component in the NDN canonical ordering.
     * @param other The other Component to compare with.
     *
     * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
     */
    bool
    operator <= (const Component& other) const { return compare(other) <= 0; }
  
    /**
     * Return true if this is less than the other Component in the NDN canonical ordering.
     * @param other The other Component to compare with.
     *
     * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
     */
    bool
    operator < (const Component& other) const { return compare(other) < 0; }

    /**
     * Return true if this is less than or equal to the other Component in the NDN canonical ordering.
     * @param other The other Component to compare with.
     *
     * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
     */
    bool
    operator >= (const Component& other) const { return compare(other) >= 0; }

    /**
     * Return true if this is greater than the other Component in the NDN canonical ordering.
     * @param other The other Component to compare with.
     *
     * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
     */
    bool
    operator > (const Component& other) const { return compare(other) > 0; }

  private:
    /**
     * FINAL_SEGMENT_PREFIX has the bytes of the component prefix used by toFinalSegment, etc.
     */
    static const uint8_t FINAL_SEGMENT_PREFIX[];
    static size_t FINAL_SEGMENT_PREFIX_LENGTH;

    Blob value_;
  }; 
  
  /**
   * Create a new Name with no components.
   */
  Name()
  : changeCount_(0)
  {
  }
  
  /**
   * Create a new Name, copying the name components.
   * @param components A vector of Component
   */
  Name(const std::vector<Component>& components)
  : components_(components), changeCount_(0)
  {
  }
  
  /**
   * Parse the uri according to the NDN URI Scheme and create the name with the components.
   * @param uri The URI string.
   */
  Name(const char* uri)
  : changeCount_(0)
  {
    set(uri);
  }
  
  /**
   * Parse the uri according to the NDN URI Scheme and create the name with the components.
   * @param uri The URI string.
   */
  Name(const std::string& uri)
  : changeCount_(0)
  {
    set(uri.c_str());
  }

  /**
   * Set the nameStruct to point to the components in this name, without copying any memory.
   * WARNING: The resulting pointers in nameStruct are invalid after a further use of this object which could reallocate memory.
   * @param nameStruct A C ndn_Name struct where the components array is already allocated.
   */
  void 
  get(struct ndn_Name& nameStruct) const;
  
  /**
   * Clear this name, and set the components by copying from the name struct.
   * @param nameStruct A C ndn_Name struct
   */
  void 
  set(const struct ndn_Name& nameStruct);
  
  /**
   * Parse the uri according to the NDN URI Scheme and set the name with the components.
   * @param uri The null-terminated URI string.
   */
  void 
  set(const char *uri);  

  /**
   * Parse the uri according to the NDN URI Scheme and set the name with the components.
   * @param uri The URI string.
   */
  void 
  set(const std::string& uri) { set(uri.c_str()); }  
  
  /**
   * Append a new component, copying from value of length valueLength.
   * @return This name so that you can chain calls to append.
   */
  Name& 
  append(const uint8_t *value, size_t valueLength) 
  {
    return append(Component(value, valueLength));
  }

  /**
   * Append a new component, copying from value.
   * @return This name so that you can chain calls to append.
   */
  Name& 
  append(const std::vector<uint8_t>& value) 
  {
    return append(value);
  }
  
  Name& 
  append(const Blob &value)
  {
    return append(value);
  }
  
  Name& 
  append(const Component &value)
  {
    components_.push_back(value);
    ++changeCount_;
    return *this;
  }
  
  /**
   * Append the components of the given name to this name.
   * @param name The Name with components to append.
   * @return This name so that you can chain calls to append.
   */
  Name&
  append(const Name& name);
  
  /**
   * @deprecated Use append.
   */
  Name& 
  appendComponent(const uint8_t *value, size_t valueLength) 
  {
    return append(value, valueLength);
  }

  /**
   * @deprecated Use append.
   */
  Name& 
  appendComponent(const std::vector<uint8_t>& value) 
  {
    return append(value);
  }
  
  /**
   * @deprecated Use append.
   */
  Name& 
  appendComponent(const Blob &value)
  {
    return append(value);
  }

  /**
   * @deprecated Use append.
   */
  Name& 
  addComponent(const uint8_t *value, size_t valueLength) 
  {
    return append(value, valueLength);
  }

  /**
   * @deprecated Use append.
   */
  Name& 
  addComponent(const std::vector<uint8_t>& value) 
  {
    return append(value);
  }
  
  /**
   * @deprecated Use append.
   */
  Name& 
  addComponent(const Blob &value)
  {
    return append(value);
  }
  
  /**
   * Clear all the components.
   */
  void 
  clear() {
    components_.clear();
    ++changeCount_;
  }
  
  /**
   * @deprecated use size().
   */
  size_t 
  getComponentCount() const { return size(); }
  
  /**
   * @deprecated Use get(i).
   */
  const Component& 
  getComponent(size_t i) const { return get(i); }
  
  /**
   * Get a new name, constructed as a subset of components.
   * @param iStartComponent The index if the first component to get.
   * @param nComponents The number of components starting at iStartComponent.
   * @return A new name.
   */
  Name
  getSubName(size_t iStartComponent, size_t nComponents) const;

  /**
   * Get a new name, constructed as a subset of components starting at iStartComponent until the end of the name.
   * @param iStartComponent The index if the first component to get.
   * @return A new name.
   */
  Name
  getSubName(size_t iStartComponent) const;
  
  /**
   * Return a new Name with the first nComponents components of this Name.
   * @param nComponents The number of prefix components.  If nComponents is -N then return the prefix up
   * to name.size() - N. For example getPrefix(-1) returns the name without the final component.
   * @return A new Name.
   */
  Name
  getPrefix(int nComponents) const
  {
    if (nComponents < 0)
      return getSubName(0, components_.size() + nComponents);
    else
      return getSubName(0, nComponents);
  }
  
  /**
   * Encode this name as a URI.
   * @return The encoded URI.
   */
  std::string 
  toUri() const;
  
  /**
   * @deprecated Use toUri().
   */
  std::string 
  to_uri() const 
  {
    return toUri();
  }

  /**
   * Append a component with the encoded segment number.
   * @param segment The segment number.
   * @return This name so that you can chain calls to append.
   */  
  Name& 
  appendSegment(uint64_t segment)
  {
    return append(Component::fromNumberWithMarker(segment, 0x00));
  }

  /**
   * Append a component with the encoded final segment number.
   * @param segment The segment number to be encoded as the final segment.
   * @return This name so that you can chain calls to append.
   */  
  Name& 
  appendFinalSegment(uint64_t segment)
  {
    return append(Component::fromNumberWithPrefix
      (segment, Component::getFinalSegmentPrefix(), Component::getFinalSegmentPrefixLength()));
  }
  
  /**
   * Append a component with the encoded version number.
   * Note that this encodes the exact value of version without converting from a time representation.
   * @param version The version number.
   * @return This name so that you can chain calls to append.
   */  
  Name& 
  appendVersion(uint64_t version)
  {
    return append(Component::fromNumberWithMarker(version, 0xFD));
  }
  
  /**
   * Check if this name has the same component count and components as the given name.
   * @param name The Name to check.
   * @return true if the names are equal, otherwise false.
   */
  bool
  equals(const Name& name) const;
  
  /**
   * Check if the N components of this name are the same as the first N components of the given name.
   * @param name The Name to check.
   * @return true if this matches the given name, otherwise false.  This always returns true if this name is empty.
   */
  bool 
  match(const Name& name) const;
  
  /**
   * Make a Blob value by decoding the escapedString between beginOffset and endOffset according to the NDN URI Scheme.
   * If the escaped string is "", "." or ".." then return a Blob with a null pointer, 
   * which means the component should be skipped in a URI name.
   * @param escapedString The escaped string.  It does not need to be null-terminated because we only scan to endOffset.
   * @param beginOffset The offset in escapedString of the beginning of the portion to decode.
   * @param endOffset The offset in escapedString of the end of the portion to decode.
   * @return The Blob value. If the escapedString is not a valid escaped component, then the Blob is a null pointer.
   */
  static Blob 
  fromEscapedString(const char *escapedString, size_t beginOffset, size_t endOffset);

  /**
   * Make a Blob value by decoding the escapedString according to the NDN URI Scheme.
   * If the escaped string is "", "." or ".." then return a Blob with a null pointer, 
   * which means the component should be skipped in a URI name.
   * @param escapedString The null-terminated escaped string.
   * @return The Blob value. If the escapedString is not a valid escaped component, then the Blob is a null pointer.
   */
  static Blob 
  fromEscapedString(const char *escapedString);

  /**
   * Make a Blob value by decoding the escapedString according to the NDN URI Scheme.
   * If the escaped string is "", "." or ".." then return a Blob with a null pointer, 
   * which means the component should be skipped in a URI name.
   * @param escapedString The escaped string.
   * @return The Blob value. If the escapedString is not a valid escaped component, then the Blob is a null pointer.
   */
  static Blob 
  fromEscapedString(const std::string& escapedString) { return fromEscapedString(escapedString.c_str()); }

  /**
   * Write the value to result, escaping characters according to the NDN URI Scheme.
   * This also adds "..." to a value with zero or more ".".
   * @param value the buffer with the value to escape
   * @param result the string stream to write to.
   */
  static void 
  toEscapedString(const std::vector<uint8_t>& value, std::ostringstream& result);

  /**
   * Convert the value by escaping characters according to the NDN URI Scheme.
   * This also adds "..." to a value with zero or more ".".
   * @param value the buffer with the value to escape
   * @return The escaped string.
   */
  static std::string
  toEscapedString(const std::vector<uint8_t>& value);

  //
  // vector equivalent interface.
  //
  
  /**
   * Get the number of components.
   * @return The number of components.
   */
  size_t 
  size() const { return components_.size(); }

  /**
   * Get the component at the given index.
   * @param i The index of the component, starting from 0.  However, if i is negative, return the component
   * at size() - (-i).
   * @return The name component at the index.
   * @throw runtime_error If index is out of range.
   */
  const Component& 
  get(int i) const;
    
  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  uint64_t 
  getChangeCount() const { return changeCount_; }
  
  /**
   * Compare this to the other Name using NDN canonical ordering.  If the first components of each name are not equal, 
   * this returns -1 if the first comes before the second using the NDN canonical ordering for name components, or 1 if it comes after.
   * If they are equal, this compares the second components of each name, etc.  If both names are the same up to
   * the size of the shorter name, this returns -1 if the first name is shorter than the second or 1 if it is longer.  
   * For example, if you std::sort gives: /a/b/d /a/b/cc /c /c/a /bb .  This is intuitive because all names
   * with the prefix /a are next to each other.  But it may be also be counter-intuitive because /c comes before /bb 
   * according to NDN canonical ordering since it is shorter.  
   * @param other The other Name to compare with.
   * @return 0 If they compare equal, -1 if *this comes before other in the canonical ordering, or
   * 1 if *this comes after other in the canonical ordering.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  int
  compare(const Name& other) const;

  const Component&
  operator [] (int i) const
  {
    return get(i);
  }

  /**
   * Append the component
   * @param component The component of type T.
   */
  template<class T> void
  push_back(const T &component)
  {
    append(component);
  }
  
  /**
   * Check if this name has the same component count and components as the given name.
   * @param name The Name to check.
   * @return true if the names are equal, otherwise false.
   */
  bool
  operator == (const Name &name) const { return equals(name); }

  /**
   * Check if this name has the same component count and components as the given name.
   * @param name The Name to check.
   * @return true if the names are not equal, otherwise false.
   */
  bool
  operator != (const Name &name) const { return !equals(name); }
  
  /**
   * Return true if this is less than or equal to the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator <= (const Name& other) const { return compare(other) <= 0; }

  /**
   * Return true if this is less than the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator < (const Name& other) const { return compare(other) < 0; }

  /**
   * Return true if this is less than or equal to the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator >= (const Name& other) const { return compare(other) >= 0; }

  /**
   * Return true if this is greater than the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator > (const Name& other) const { return compare(other) > 0; }

  /**
   * @deprecated Use operator < .
   */
  static bool
  breadthFirstLess(const Name& name1, const Name& name2) { return name1 < name2; }
  
  /**
   * @deprecated Not needed since Name defines operator < .
   */
  struct BreadthFirstLess {
    bool operator() (const Name& name1, const Name& name2) const { return name1 < name2; }
  };
  
  //
  // Iterator interface to name components.
  //
  typedef std::vector<Component>::const_iterator const_iterator;
  typedef std::vector<Component>::const_reverse_iterator const_reverse_iterator;

  typedef Component partial_type;

  /**
   * Begin iterator (const).
   */
  const_iterator
  begin() const { return components_.begin(); }

  /**
   * End iterator (const).
   */
  const_iterator
  end() const { return components_.end(); }

  /**
   * Reverse begin iterator (const).
   */
  const_reverse_iterator
  rbegin() const { return components_.rbegin(); }

  /**
   * Reverse end iterator (const).
   */
  const_reverse_iterator
  rend() const { return components_.rend(); }

private:
  std::vector<Component> components_;
  uint64_t changeCount_;
};  

inline std::ostream&
operator << (std::ostream& os, const Name& name)
{
  os << name.toUri();
  return os;
}

}

#endif

