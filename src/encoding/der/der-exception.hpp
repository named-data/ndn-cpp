/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_DER_EXCEPTION_HPP
#define NDN_DER_EXCEPTION_HPP

#include <exception>
#include <string>

namespace ndn {

class DerException : public std::exception {
public:
  DerException(const std::string& errorMessage) throw();

  ~DerException() throw();

  std::string Msg() { return errorMessage_; }

  virtual const char* what() const throw() { return errorMessage_.c_str(); }

private:
  const std::string errorMessage_;
};

class NegativeLengthException : public DerException
{
public:
  NegativeLengthException(const std::string& errorMessage)
  : DerException(errorMessage)
  {}
};

class DerEncodingException : public DerException
{
public:
  DerEncodingException(const std::string& errorMessage)
  : DerException(errorMessage)
  {}
};

class DerDecodingException : public DerException
{
public:
  DerDecodingException(const std::string& errorMessage)
  : DerException(errorMessage)
  {}
};

}

#endif
