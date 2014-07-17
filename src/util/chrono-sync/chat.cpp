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

#include <sstream>
#include "../../c/util/crypto.h"
#include "../../c/util/time.h"
#include "../logging.hpp"
#include "sync-state.pb.h"
#include "chatbuf.pb.h"
#include "chat.hpp"

using namespace std;

using namespace ndn::func_lib;
#if NDN_CPP_HAVE_STD_FUNCTION && NDN_CPP_WITH_STD_FUNCTION
// In the std library, the placeholders are in a different namespace than boost.
using namespace func_lib::placeholders;
#endif

namespace ndn {

Chat::Chat
  (const string& screenName, const string& chatRoom,
   const std::string& localPrefix, Transport& transport, Face& face,
   KeyChain& keyChain, const Name& certificateName)
  : screen_name_(screenName), chatroom_(chatRoom), maxmsgcachelength_(100),
    isRecoverySyncState_(true), sync_lifetime_(5000.0), face_(face),
    keyChain_(keyChain), certificateName_(certificateName)
{
  // This should only be called once, so get the random string here.
  chat_prefix_ = localPrefix + "/" + chatroom_ + "/" + Chat::getRandomString();
  int session = (int)::round(ndn_getNowMilliseconds()  / 1000.0);
  ostringstream tempStream;
  tempStream << screen_name_ << session;
  usrname_ = tempStream.str();
  sync_.reset(new ChronoSync
    (bind(&Chat::sendInterest, this, _1, _2),
     bind(&Chat::initial, this), chat_prefix_,
     Name("/ndn/broadcast/ChronoChat-0.3").append(chatroom_), session,
     transport, face, keyChain, certificateName, sync_lifetime_,
     onRegisterFailed));

  face.registerPrefix
    (Name(chat_prefix_), bind(&Chat::onInterest, this, _1, _2, _3, _4),
     onRegisterFailed);
  _LOG_DEBUG("data prefix registered.");
}

void
Chat::initial()
{
  Chat& self = *this;
  //_LOG_DEBUG("initial chat");
#if 0 // TODO: Set the heartbeat timeout.
  var myVar = setInterval(function(){self.heartbeat();},60000);
#endif
  if (find(roster_.begin(), roster_.end(), usrname_) == roster_.end()) {
    roster_.push_back(usrname_);
#if 0 // TODO: Show the Member.
    document.getElementById('menu').innerHTML = '<p><b>Member</b></p>';
    document.getElementById('menu').innerHTML += '<ul><li>'+screen_name_+'</li></ul>';
#else
    cout << "Member: " << screen_name_ << endl;
#endif
#if 0 // TODO: Show the join message.
    var d = new Date();
    var t = d.getTime();
    document.getElementById('txt').innerHTML += '<div><b><grey>'+screen_name_+'-'+d.toLocaleTimeString()+': Join</grey></b><br /></div>'
    var objDiv = document.getElementById("txt");
    objDiv.scrollTop = objDiv.scrollHeight;
#else
    cout << screen_name_ << ": Join" << endl;
#endif
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_JOIN, "xxx");
  }
}

void
Chat::sendInterest
  (const google::protobuf::RepeatedPtrField<Sync::SyncState >& content,
   bool isRecovery)
{
  // This is used by onData to decide whether to display the chat messages.
  isRecoverySyncState_ = isRecovery;

  //_LOG_DEBUG(content);
  vector<string> sendlist;
  vector<int> sessionlist;
  vector<int> seqlist;
  for (size_t j = 0; j < content.size(); ++j) {
    if (content.Get(j).type() == 0) {
      Name name_components(content.Get(j).name());
      string name_t = name_components.get(-1).toEscapedString();
      int session = content.Get(j).seqno().session();
      if (name_t != screen_name_) {
        int index_n = -1;
        for (size_t k = 0; k < sendlist.size(); ++k) {
          if (sendlist[k] == content.Get(j).name()) {
            index_n = k;
            break;
          }
        }
        if (index_n != -1) {
          sessionlist[index_n] = session;
          seqlist[index_n] = content.Get(j).seqno().seq();
        }
        else{
          sendlist.push_back(content.Get(j).name());
          sessionlist.push_back(session);
          seqlist.push_back(content.Get(j).seqno().seq());
        }
      }
    }
  }

  for (size_t i = 0; i < sendlist.size(); ++i) {
    ostringstream uri;
    uri << sendlist[i] << "/" << sessionlist[i] << "/" << seqlist[i];
    Interest interest(uri.str());
    interest.setInterestLifetimeMilliseconds(sync_lifetime_);
    face_.expressInterest
      (interest, bind(&Chat::onData, this, _1, _2),
       bind(&Chat::chatTimeout, this, _1));
    //_LOG_DEBUG(interest.getName().toUri());
    //_LOG_DEBUG("Chat Interest expressed.");
  }
}

void
Chat::onInterest
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& inst, Transport& transport,
   uint64_t registeredPrefixId)
{
  //_LOG_DEBUG("Chat Interest received in callback.");
  //_LOG_DEBUG(inst->getName().toUri());
  SyncDemo::ChatMessage content;
  int seq = ::atoi(inst->getName().get(6).toEscapedString().c_str());
  for (int i = msgcache_.size() - 1; i >= 0; --i) {
    if (msgcache_[i]->getSequenceNo() == seq) {
      if (msgcache_[i]->getMessageType() != SyncDemo::ChatMessage_ChatMessageType_CHAT) {
        content.set_from(screen_name_);
        content.set_to(chatroom_);
        content.set_type((SyncDemo::ChatMessage_ChatMessageType)msgcache_[i]->getMessageType());
        content.set_timestamp(::round(msgcache_[i]->getTime() / 1000.0));
      }
      else {
        content.set_from(screen_name_);
        content.set_to(chatroom_);
        content.set_type((SyncDemo::ChatMessage_ChatMessageType)msgcache_[i]->getMessageType());
        content.set_data(msgcache_[i]->getMessage());
        content.set_timestamp(::round(msgcache_[i]->getTime() / 1000.0));
      }
      break;
    }
  }

  if (content.from().size() != 0) {
    ptr_lib::shared_ptr<vector<uint8_t> > array(new vector<uint8_t>(content.ByteSize()));
    content.SerializeToArray(&array->front(), array->size());
    Data co(inst->getName());
    co.setContent(Blob(array, false));
    keyChain_.sign(co, certificateName_);
    try {
      transport.send(*co.wireEncode());
      //_LOG_DEBUG(content.SerializeAsString());
    }
    catch (std::exception& e) {
      _LOG_DEBUG(e.what());
    }
  }
}

void
Chat::onData
  (const ptr_lib::shared_ptr<const Interest>& inst,
   const ptr_lib::shared_ptr<Data>& co)
{
  //_LOG_DEBUG("Chat ContentObject received in callback");
  //_LOG_DEBUG("name: " + co->getName().toUri());
  SyncDemo::ChatMessage content;
  content.ParseFromArray(co->getContent().buf(), co->getContent().size());
  if (ndn_getNowMilliseconds() - content.timestamp() * 1000.0 < 120000.0) {
    string name = content.from();
    Name name_t(name);
    Name prefix = name_t.getSubName(1, 5).toUri();
    int session = ::atoi(co->getName().get(5).toEscapedString().c_str());
    int seqno = ::atoi(co->getName().get(6).toEscapedString().c_str());
    ostringstream tempStream;
    tempStream << name << session;
    string nameAndSession = tempStream.str();

    size_t l = 0;
    //update roster
    while (l < roster_.size()) {
      string name_t2 = roster_[l].substr(0, roster_[l].size() - 10);
      int session_t = ::atoi(roster_[l].substr(roster_[l].size() - 10, 10).c_str());
      if (name != name_t2 && content.type() != 2)
        ++l;
      else {
        if (name == name_t2 && session > session_t)
          roster_[l] = nameAndSession;
        break;
      }
    }

    if (l == roster_.size()) {
      roster_.push_back(nameAndSession);
#if 0 // TODO: Show the join event.
      var t = (new Date(content.timestamp*1000)).toLocaleTimeString();
      document.getElementById('txt').innerHTML += '<div><b><grey>'+name+'-'+t+': Join'+'</grey></b><br /></div>';
      var objDiv = document.getElementById("txt");
      objDiv.scrollTop = objDiv.scrollHeight;
#else
      cout << name << ": Join" << endl;
#endif
#if 0 // TODO: Show the new roster.
      document.getElementById('menu').innerHTML = '<p><b>Member</b></p><ul>';
      for (size_t i = 0; i < roster_.size(); ++i) {
        string name_t = roster_[i].substring(0, roster_[i].size() - 10);
        document.getElementById('menu').innerHTML += '<li>'+name_t+'</li>';
      }
      document.getElementById('menu').innerHTML += '</ul>';
#endif
    }

    Chat& self = *this;
#if 0 // TODO: Set the alive timeout.
    setTimeout(function(){self.alive(seqno,name,session,prefix);},120000);
#endif
    // isRecoverySyncState_ was set by sendInterest.
    // TODO: If isRecoverySyncState_ changed, this assumes that we won't
    //   data from an interest sent before it changed.
    if (content.type() == 0 && !isRecoverySyncState_ && content.from() != screen_name_) {
#if 0 // TODO: Show the message.
      // The display on the screen will not display old data.
      var escaped_msg = $('<div/>').text(content.data).html();  // encode special html characters to avoid script injection
      document.getElementById('txt').innerHTML +='<p><grey>'+ content.from+'-'+t+':</grey><br />'+escaped_msg+'</p>';
      var objDiv = document.getElementById("txt");
      objDiv.scrollTop = objDiv.scrollHeight;
#else
      cout << content.from() << ": " << content.data() << endl;
#endif
    }
    else if (content.type() == 2) {
      // leave message
      vector<string>::iterator n = find(roster_.begin(), roster_.end(), nameAndSession);
      if (n != roster_.end() && name != screen_name_) {
        roster_.erase(n);
#if 0 // TODO: Update the roster.
        document.getElementById('menu').innerHTML = '<p><b>Member</b></p><ul>';
        for (var i = 0; i < roster.size(); ++i) {
          string name_t = roster_[i].substring(0, roster_[i].size() - 10);
          document.getElementById('menu').innerHTML += '<li>'+name_t+'</li>';
        }
        document.getElementById('menu').innerHTML += '</ul>';
#endif
        //_LOG_DEBUG(name + " leave");
#if 0 // TODO: Show leave message.
        var d = new Date(content.timestamp*1000);
        var t = d.toLocaleTimeString();
        document.getElementById('txt').innerHTML += '<div><b><grey>'+name+'-'+t+': Leave</grey></b><br /></div>'
        var objDiv = document.getElementById("txt");
        objDiv.scrollTop = objDiv.scrollHeight;
#else
        cout << name << ": Leave" << endl;
#endif
      }
    }
  }
}

void
Chat::chatTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{
  //_LOG_DEBUG("no chat data coming back");
}

void
Chat::heartbeat()
{
  if (msgcache_.size() == 0)
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_JOIN, "xxx");

  sync_->publishNextSequenceNo();
  messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_HELLO, "xxx");

  //_LOG_DEBUG("Heartbeat Interest expressed.");
}

void
Chat::sendMessage(const string& chatmsg)
{
  if (msgcache_.size() == 0)
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_JOIN, "xxx");

  // forming Sync Data Packet.
  if (chatmsg != "") {
#if 0 // TODO: Clear fname (why?)
    document.getElementById('fname').value = "";
#endif
    sync_->publishNextSequenceNo();
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_CHAT, chatmsg);

    //_LOG_DEBUG("Sync Interest expressed.");
#if 0 // TODO: Show the message.
    var tt = d.toLocaleTimeString();
    var escaped_msg = $('<div/>').text(msg).html();  // encode special html characters to avoid script injection
    document.getElementById('txt').innerHTML += '<p><grey>'+ screen_name_+'-'+tt+':</grey><br />'+ escaped_msg + '</p>';
    var objDiv = document.getElementById("txt");
    objDiv.scrollTop = objDiv.scrollHeight;
#else
    cout << screen_name_ << ": " << chatmsg << endl;
#endif
  }
  else
    _LOG_DEBUG("message cannot be empty");
}

void
Chat::leave()
{
  //_LOG_DEBUG("Leaving the Chatroom...");
#if 0 // TODO: Hide the chat area.
  $("#chat").hide();
  document.getElementById('room').innerHTML = 'Please close the window. Thank you';
#endif
  sync_->publishNextSequenceNo();
  messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_LEAVE, "xxx");
}

void
Chat::alive
  (int temp_seq, const string& name, int session, const string& prefix)
{
  //_LOG_DEBUG("check alive");
  int seq = sync_->getProducerSequenceNo(prefix, session);
  ostringstream tempStream;
  tempStream << name << session;
  string nameAndSession = tempStream.str();
  vector<string>::iterator n = find(roster_.begin(), roster_.end(), nameAndSession);
  if (seq != -1 && n != roster_.end()) {
    if (temp_seq == seq){
      roster_.erase(n);
      //_LOG_DEBUG(name + " leave");
#if 0 // TODO: Show the leave message. Update the roster.
      var d = new Date();
      var t = d.toLocaleTimeString();
      document.getElementById('txt').innerHTML += '<div><b><grey>'+name+'-'+t+': Leave</grey></b><br /></div>'
      var objDiv = document.getElementById("txt");
      objDiv.scrollTop = objDiv.scrollHeight;
      document.getElementById('menu').innerHTML = '<p><b>Member</b></p><ul>';
      for(size_t i = 0; i < roster_.size(); ++i) {
        string name_t = roster_[i].substring(0, roster_[i].size() - 10);
        document.getElementById('menu').innerHTML += '<li>'+name_t+'</li>';
      }
      document.getElementById('menu').innerHTML += '</ul>';
#else
      cout << name << ": Leave" << endl;
#endif
    }
  }
}

void
Chat::messageCacheAppend(int messageType, const string& message)
{
  msgcache_.push_back(ptr_lib::make_shared<CachedMessage>
    (sync_->getSequenceNo(), messageType, message, ndn_getNowMilliseconds()));
  while (msgcache_.size() > maxmsgcachelength_)
    msgcache_.erase(msgcache_.begin());
}

string
Chat::getRandomString()
{
  string seed("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789");
  string result;
  uint8_t random[10];
  ndn_generateRandomBytes(random, sizeof(random));
  for (int i = 0; i < 10; ++i) {
    // Using % means the distribution isn't uniform, but that's OK.
    size_t pos = (size_t)random[i] % seed.size();
    result += seed[pos];
  }

  return result;
}

void
Chat::onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
{
  _LOG_DEBUG("Register failed for prefix " + prefix->toUri());
}

}
