/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_CONTROL_PARAMETERS_OPTIONS_H
#define NDN_CONTROL_PARAMETERS_OPTIONS_H

#include <ndn-cpp/c/common.h>
#include <ndn-cpp/c/forwarding-flags.h>
#include "name.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ndn_NfdForwardingFlags_CHILD_INHERIT = 1,
  ndn_NfdForwardingFlags_CAPTURE       = 2,
} ndn_NfdForwardingFlags;

/**
 * An ndn_ControlParameters holds fields for a ControlParameters which is 
 * used in the command interest such as to register a prefix with a forwarder.
 */
struct ndn_ControlParameters {
  struct ndn_Name name;
  int faceId;                        /**< -1 for none. */
  // TODO: Add "Uri" string.
  int localControlFeature;           /**< -1 for none. */
  int origin;                        /**< -1 for none. */
  int cost;                          /**< -1 for none. */
  struct ndn_ForwardingFlags flags;
  // TODO: Add "Strategy" name.
  ndn_Milliseconds expirationPeriod; /**< -1 for none. */
};

/**
 * Initialize an ndn_ControlParameters struct with the pre-allocated 
 * nameComponents, and defaults for all the values.
 * @param self A pointer to the ndn_ControlParameters struct.
 * @param nameComponents the pre-allocated array of ndn_NameComponent.
 * @param maxNameComponents the number of elements in the allocated 
 * nameComponents array.
 */
static inline void 
ndn_ControlParameters_initialize
  (struct ndn_ControlParameters *self, struct ndn_NameComponent *nameComponents, 
   size_t maxNameComponents) 
{
  ndn_Name_initialize(&self->name, nameComponents, maxNameComponents);
  self->faceId = -1;
  self->localControlFeature = -1;
  self->origin = -1;
  self->cost = -1;
  ndn_ForwardingFlags_initialize(&self->flags);
  self->expirationPeriod = -1.0;
}

#ifdef __cplusplus
}
#endif

#endif
