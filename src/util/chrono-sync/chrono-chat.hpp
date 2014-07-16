/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from ChronoChat-js by Qiuhan Ding and Wentao Shang.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_CHRONO_CHAT_HPP
#define NDN_CHRONO_CHAT_HPP

#include <string>
#include <ndn-cpp/transport/tcp-transport.hpp>
#include <ndn-cpp/util/chrono-sync/chrono-sync.hpp>

namespace ndn {

class Chat;

class ChronoChat {
public:
  static void
  start(const char* screenName, const char* chatRoom, const char* hub);

  static Chat* chat;
  static ptr_lib::shared_ptr<Face> face;

private:
  static void
  prefixData
    (const ptr_lib::shared_ptr<const Interest>& inst,
     const ptr_lib::shared_ptr<Data>& co);

  static void
  prefixTimeOut(const ptr_lib::shared_ptr<const Interest>& inst);

  static void onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix);

  static ptr_lib::shared_ptr<Transport> transport;
  static ptr_lib::shared_ptr<KeyChain> keyChain;
  static Name certificateName;
  static std::string screenName;
  static std::string chatRoom;
};

}

#endif
