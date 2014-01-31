/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_EXCLUDE_HPP
#define NDN_EXCLUDE_HPP

#include "name.hpp"
#include "c/interest-types.h"

struct ndn_ExcludeEntry;
struct ndn_Exclude;

namespace ndn {

/**
 * An Exclude holds a vector of Exclude::Entry.
 */
class Exclude {
public:
  /**
   * Create a new Exclude with no entries.
   */
  Exclude() 
  : changeCount_(0)
  {
  }

  /**
   * An Exclude::Entry holds an ndn_ExcludeType, and if it is a COMPONENT, it holds the component value.
   */
  class Entry {
  public:
    /**
     * Create an Exclude::Entry of type ndn_Exclude_ANY
     */
    Entry()
    : type_(ndn_Exclude_ANY)
    {    
    }

    /**
     * Create an Exclude::Entry of type ndn_Exclude_COMPONENT.
     */
    Entry(const uint8_t *component, size_t componentLen) 
    : type_(ndn_Exclude_COMPONENT), component_(component, componentLen)
    {
    }

    /**
     * Create an Exclude::Entry of type ndn_Exclude_COMPONENT.
     */
    Entry(const Blob& component) 
    : type_(ndn_Exclude_COMPONENT), component_(component)
    {
    }

    /**
     * Set the type in the excludeEntryStruct and to point to this entry, without copying any memory.
     * WARNING: The resulting pointer in excludeEntryStruct is invalid after a further use of this object which could reallocate memory.
     * @param excludeEntryStruct the C ndn_ExcludeEntry struct to receive the pointer
     */
    void 
    get(struct ndn_ExcludeEntry& excludeEntryStruct) const;

    ndn_ExcludeType 
    getType() const { return type_; }

    const Name::Component& 
    getComponent() const { return component_; }

  private:
    ndn_ExcludeType type_;
    Name::Component component_; /**< only used if type_ is ndn_Exclude_COMPONENT */
  }; 

  /**
   * Get the number of entries.
   * @return The number of entries.
   */
  size_t 
  size() const { return entries_.size(); }
  
  /**
   * Get the entry at the given index.
   * @param i The index of the entry, starting from 0.
   * @return The entry at the index.
   */
  const Exclude::Entry& 
  get(size_t i) const { return entries_[i]; }

  /**
   * @deprecated Use size().
   */  
  size_t 
  getEntryCount() const { return entries_.size(); }
  
  /**
   * @deprecated Use get(i).
   */  
  const Exclude::Entry& 
  getEntry(size_t i) const { return entries_[i]; }
  
  /**
   * Set the excludeStruct to point to the entries in this Exclude, without copying any memory.
   * WARNING: The resulting pointers in excludeStruct are invalid after a further use of this object which could reallocate memory.
   * @param excludeStruct a C ndn_Exclude struct where the entries array is already allocated
   */
  void 
  get(struct ndn_Exclude& excludeStruct) const;
  
  /**
   * Clear this Exclude, and set the entries by copying from the ndn_Exclude struct.
   * @param excludeStruct a C ndn_Exclude struct
   */
  void 
  set(const struct ndn_Exclude& excludeStruct);

  /**
   * Append a new entry of type ndn_Exclude_ANY.
   * @return This Exclude so that you can chain calls to append.
   */
  Exclude& 
  appendAny()
  {    
    entries_.push_back(Entry());
    ++changeCount_;
    return *this;
  }
  
  /**
   * Append a new entry of type ndn_Exclude_COMPONENT, copying from component of length componentLength.
   * @param component A pointer to the component byte array.
   * @param componentLength The length of component.
   * @return This Exclude so that you can chain calls to append.
   */
  Exclude& 
  appendComponent(const uint8_t *component, size_t componentLength) 
  {
    entries_.push_back(Entry(component, componentLength));
    ++changeCount_;
    return *this;
  }

  /**
   * Append a new entry of type ndn_Exclude_COMPONENT, taking another pointer to the Blob value.
   * @param component A blob with a pointer to an immutable array.  The pointer is copied.
   * @return This Exclude so that you can chain calls to append.
   */
  Exclude& 
  appendComponent(const Blob &component) 
  {
    entries_.push_back(Entry(component));
    ++changeCount_;
    return *this;
  }

  /**
   * @deprecated Use appendAny.
   */
  Exclude& 
  addAny() { return appendAny(); }

  /**
   * @deprecated Use appendComponent.
   */
  Exclude& 
  addComponent(uint8_t *component, size_t componentLength) { return appendComponent(component, componentLength); }
  
  /**
   * Clear all the entries.
   */
  void 
  clear() 
  {
    entries_.clear();
    ++changeCount_;
  }
  
  /**
   * Encode this Exclude with elements separated by "," and ndn_Exclude_ANY shown as "*".
   * @return the URI string
   */
  std::string 
  toUri() const;

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  uint64_t 
  getChangeCount() const { return changeCount_; }
  
private:
  std::vector<Entry> entries_;
  uint64_t changeCount_;
};

}

#endif