/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
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

#ifndef NDN_PREFIX_DISCOVERY_HPP
#define NDN_PREFIX_DISCOVERY_HPP

#include <ndn-cpp/face.hpp>

namespace ndntools {

/**
 * PrefixDiscovery repeatedly sends an interest for the routable prefix to
 * the connected NFD. When a new prefix Name (or Names) is received, this
 * calls an application callback. Note that NFD must be configured to respond
 * to an interest for the routable prefix, for example by running
 * ndn-autoconfig-server. See:
 * https://named-data.net/doc/NFD/current/misc/local-prefix-discovery.html
 */
class PrefixDiscovery {
public:
  typedef ndn::func_lib::function<void
    (const ndn::ptr_lib::shared_ptr<std::vector<ndn::Name> >& prefixes)> OnPrefixes;

  /**
   * Create a PrefixDiscovery to use the given OnPrefixes callback and face.
   * After creating an instance of PrefixDiscovery, you must call start().
   * @param onPrefixes When a new list of prefixes is received (or the first
   * list), this calls onPrefixes(prefixes) with the list of prefix Names. (It
   * is normal if this list only has one prefix.)
   * @param face The Face for calling expressInterest to request the list of
   * prefixes. Usually the Face should be connected to the local NFD.
   * @param periodMilliseconds (optional) The period in milliseconds between
   * each interest to request the current list of prefixes. If omitted, use the
   * default 60000 (one minute).
   */
  PrefixDiscovery
    (OnPrefixes onPrefixes, ndn::Face* face,
     ndn::Milliseconds periodMilliseconds = 60000.0)
  : impl_(new Impl(onPrefixes, face, periodMilliseconds))
  {
  }

  /**
   * Start the process by sending the first interest to the Face given to the
   * constructor, then automatically repeat indefinitely with the period
   * specified in the constructor. If any Interest times out, log an event and
   * send another Interest to try again. If the process is already started, do
   * nothing.
   */
  void
  start() { impl_->start(); }

  /**
   * Stop the process. You can restart by calling start() again.
   */
  void
  stop() { impl_->stop(); }

private:
  /**
   * PrefixDiscovery::Impl does the work of PrefixDiscovery. It is a separate
   * class so that PrefixDiscovery can create an instance in a shared_ptr to
   * use in callbacks.
   */
  class Impl : public ndn::ptr_lib::enable_shared_from_this<Impl> {
  public:
    Impl(OnPrefixes onPrefixes, ndn::Face* face,
         ndn::Milliseconds periodMilliseconds)
    : onPrefixes_(onPrefixes), face_(face),
      periodMilliseconds_(periodMilliseconds), isEnabled_(false)
    {
    }

    void
    start()
    {
      if (isEnabled_)
        // Already started.
        return;

      isEnabled_ = true;
      express();
    }

    void stop() { isEnabled_ = false; }

  private:
    void
    express();

    void
    onData
      (const ndn::ptr_lib::shared_ptr<const ndn::Interest>& interest,
       const ndn::ptr_lib::shared_ptr<ndn::Data>& data);

    void
    onTimeout(const ndn::ptr_lib::shared_ptr<const ndn::Interest>& interest);

    OnPrefixes onPrefixes_;
    ndn::Face* face_;
    ndn::Milliseconds periodMilliseconds_;
    std::vector<ndn::Name> prefixes_;
    bool isEnabled_;
  };

  ndn::ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
