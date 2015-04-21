/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#ifndef NDN_ELEMENT_LISTENER_HPP
#define NDN_ELEMENT_LISTENER_HPP

#include "../c/encoding/element-listener.h"

namespace ndn {

/**
 * An ElementListener extends an ndn_ElementListener struct to proved an abstract virtual onReceivedElement function which wraps
 * the onReceivedElement used by the ndn_ElementListener struct.  You must extend this class to override onReceivedElement.
 */
class ElementListener : public ndn_ElementListener {
public:
  ElementListener()
  {
    ndn_ElementListener_initialize(this, staticOnReceivedElement);
  }

  /**
   * This is called when an entire binary XML element is received.  You must extend this class to override this method.
   * @param element pointer to the binary XML element.  This buffer is only valid during this call.  If you need the data
   * later, you must copy.
   * @param elementLength length of element
   */
  virtual void
  onReceivedElement(const uint8_t *element, size_t elementLength) = 0;

private:
  /**
   * Call the virtual method onReceivedElement. This is used to initialize the base ndn_ElementListener struct.
   * @param self
   * @param element
   * @param elementLength
   */
  static void
  staticOnReceivedElement(struct ndn_ElementListener *self, const uint8_t *element, size_t elementLength);
};

}

#endif
