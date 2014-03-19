/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "util/ndn_memory.h"
#include "interest.h"

int ndn_Exclude_compareComponents(struct ndn_NameComponent *component1, struct ndn_NameComponent *component2)
{
  if (component1->value.length < component2->value.length)
    return -1;
  if (component1->value.length > component2->value.length)
    return 1;

  // The components are equal length.  Just do a byte compare.  
  return ndn_memcmp(component1->value.value, component2->value.value, component1->value.length);
}
