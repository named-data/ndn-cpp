/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/prefix-registration-options.hpp>
#include "c/prefix-registration-options.h"

using namespace std;

namespace ndn {

void 
PrefixRegistrationOptions::get
  (struct ndn_PrefixRegistrationOptions& prefixRegistrationOptionsStruct) const 
{
  prefix_.get(prefixRegistrationOptionsStruct.prefix);
  prefixRegistrationOptionsStruct.faceId = faceId_;
  prefixRegistrationOptionsStruct.forwardingFlags = forwardingFlags_;
  prefixRegistrationOptionsStruct.cost = cost_;
  prefixRegistrationOptionsStruct.expirationPeriod = expirationPeriod_;
}

void 
PrefixRegistrationOptions::set
  (const struct ndn_PrefixRegistrationOptions& prefixRegistrationOptionsStruct) 
{
  prefix_.set(prefixRegistrationOptionsStruct.prefix);
  faceId_ = prefixRegistrationOptionsStruct.faceId;
  forwardingFlags_ = prefixRegistrationOptionsStruct.forwardingFlags;
  cost_ = prefixRegistrationOptionsStruct.cost;
  expirationPeriod_ = prefixRegistrationOptionsStruct.expirationPeriod;
}

}
