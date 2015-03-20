/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

#ifndef NDN_ELEMENT_LISTENER_LITE_HPP
#define NDN_ELEMENT_LISTENER_LITE_HPP

#include "../../c/encoding/element-reader-types.h"

namespace ndn {

/**
 * ElementListenerLite is a base class for you to override to implement
 * onReceivedElement.
 * @param onReceivedElement When an entire packet element is received, call
 * onReceivedElement(self, element, elementLength) where self is the pointer to
 * this ElementListenerLite object, and element is a pointer to the array of
 * length elementLength with the bytes of the element.
 */
class ElementListenerLite : private ndn_ElementListener {
public:
  ElementListenerLite();

  /**
   * onReceivedElement is called when an entire packet element is received.
   * @param element A pointer to the array with the bytes of the element.
   * @param elementLength The number of bytes in element.
   */
  virtual void
  onReceivedElement(uint8_t *element, size_t elementLength) = 0;

private:
  // Declare friends who can downcast to the private base.
  friend class TcpTransportLite;
  friend class UdpTransportLite;
  friend class UnixTransportLite;

  /**
   * This the static onReceivedElement to pass to ndn_ElementListener_initialize
   * which will call the virtual onReceivedElement method.
   * @param self A pointer to this object.
   * @param element A pointer to the array with the bytes of the element.
   * @param elementLength The number of bytes in element.
   */
  static void
  onReceivedElementWrapper
    (ndn_ElementListener *self, uint8_t *element, size_t elementLength);
};

}

#endif
