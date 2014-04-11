/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_PREFIX_REGISTRATION_OPTIONS_H
#define NDN_PREFIX_REGISTRATION_OPTIONS_H

#include <ndn-cpp/c/common.h>
#include <ndn-cpp/c/forwarding-flags.h>
#include "name.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ndn_NrdForwardingFlags_CHILD_INHERIT = 1,
  ndn_NrdForwardingFlags_CAPTURE       = 2,
} ndn_NrdForwardingFlags;

/**
 * An ndn_PrefixRegistrationOptions holds fields for a PrefixRegOptions which is 
 * used in the command interest to register a prefix with a forwarder.
 */
struct ndn_PrefixRegistrationOptions {
  struct ndn_Name prefix;
  int faceId;                        /**< -1 for none. */
  struct ndn_ForwardingFlags flags;
  int cost;                          /**< -1 for none. */
  ndn_Milliseconds expirationPeriod; /**< -1 for none. */
  // TODO: Add "Protocol" string.
};

/**
 * Initialize an ndn_PrefixRegistrationOptions struct with the pre-allocated 
 * prefixNameComponents, and defaults for all the values.
 * @param self A pointer to the ndn_PrefixRegistrationOptions struct.
 * @param prefixNameComponents the pre-allocated array of ndn_NameComponent.
 * @param maxPrefixNameComponents the number of elements in the allocated 
 * prefixNameComponents array.
 */
static inline void 
ndn_PrefixRegistrationOptions_initialize
  (struct ndn_PrefixRegistrationOptions *self, 
   struct ndn_NameComponent *prefixNameComponents, 
   size_t maxPrefixNameComponents) 
{
  ndn_Name_initialize(&self->prefix, prefixNameComponents, maxPrefixNameComponents);
  self->faceId = -1;
  ndn_ForwardingFlags_initialize(&self->flags);
  self->cost = -1;
  self->expirationPeriod = -1.0;
}

#ifdef __cplusplus
}
#endif

#endif
