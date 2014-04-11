/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_PREFIX_REGISTRATION_OPTIONS_HPP
#define NDN_PREFIX_REGISTRATION_OPTIONS_HPP

#include "name.hpp"
#include "forwarding-flags.hpp"
#include "encoding/wire-format.hpp"

struct ndn_PrefixRegistrationOptions;

namespace ndn {

/**
 * A PrefixRegistrationOptions holds a Name prefix and other fields for a 
 * PrefixRegOptions which is used in the command interest to register a prefix 
 * with a forwarder.
 */
class PrefixRegistrationOptions {
public:    
  PrefixRegistrationOptions(const Name& prefix) 
  : prefix_(prefix), faceId_(-1), cost_(-1), expirationPeriod_(-1.0)
  {
  }

  PrefixRegistrationOptions()
  : faceId_(-1), cost_(-1), expirationPeriod_(-1.0)
  {
  }
  
  Blob 
  wireEncode(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const 
  {
    return wireFormat.encodePrefixRegistrationOptions(*this);
  }
  
  void 
  wireDecode(const uint8_t *input, size_t inputLength, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) 
  {
    wireFormat.decodePrefixRegistrationOptions(*this, input, inputLength);
  }
  
  void 
  wireDecode(const std::vector<uint8_t>& input, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) 
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }
  
  /**
   * Set the prefixRegistrationOptionsStruct to point to the components in this 
   * prefix registration options, without copying any memory.
   * WARNING: The resulting pointers in prefixRegistrationOptionsStruct are 
   * invalid after a further use of this object which could reallocate memory.
   * @param prefixRegistrationOptionsStruct a C ndn_PrefixRegistrationOptions 
   * struct where the prefix name components array is already allocated.
   */
  void 
  get(struct ndn_PrefixRegistrationOptions& prefixRegistrationOptionsStruct) const;

  Name& 
  getPrefix() { return prefix_; }
  
  const Name& 
  getPrefix() const { return prefix_; }
  
  int 
  getFaceId() const { return faceId_; }

  const ForwardingFlags& 
  getFlags() const { return flags_; }
  
  int 
  getCost() const { return cost_; }

  Milliseconds 
  getExpirationPeriod() const { return expirationPeriod_; }
  
  /**
   * Clear this prefix registration options, and set the values by copying from 
   * prefixRegistrationOptionsStruct.
   * @param prefixRegistrationOptionsStruct a C ndn_PrefixRegistrationOptions 
   * struct.
   */
  void 
  set(const struct ndn_PrefixRegistrationOptions& prefixRegistrationOptionsStruct);
  
  void 
  setFaceId(int faceId) { faceId_ = faceId; }
      
  void 
  setFlags(const ForwardingFlags& flags) { flags_ = flags; }
      
  void 
  setCost(int cost) { cost_ = cost; }
      
  void 
  setExpirationPeriod(Milliseconds expirationPeriod) 
  { 
    expirationPeriod_ = expirationPeriod; 
  }
      
private:
  Name prefix_;
  int faceId_;                    /**< -1 for none. */
  ForwardingFlags flags_;
  int cost_;                      /**< -1 for none. */
  Milliseconds expirationPeriod_; /**< -1 for none. */
  // TODO: Add "Protocol" string.
};

}

#endif
