/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From PyNDN der.py by Adeola Bannis <thecodemaiden@gmail.com>.
 * @author: Originally from code in ndn-cxx by Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_DER_NODE_TYPE_HPP
#define NDN_DER_NODE_TYPE_HPP

namespace ndn {

/**
 * The DerNodeType enum defines the known DER node types.
 */
enum DerNodeType {
  DerNodeType_Eoc = 0,
  DerNodeType_Boolean = 1,
  DerNodeType_Integer = 2,
  DerNodeType_BitString = 3,
  DerNodeType_OctetString = 4,
  DerNodeType_Null = 5,
  DerNodeType_ObjectIdentifier = 6,
  DerNodeType_ObjectDescriptor = 7,
  DerNodeType_External = 40,
  DerNodeType_Real = 9,
  DerNodeType_Enumerated = 10,
  DerNodeType_EmbeddedPdv = 43,
  DerNodeType_Utf8String = 12,
  DerNodeType_RelativeOid = 13,
  DerNodeType_Sequence = 48,
  DerNodeType_Set = 49,
  DerNodeType_NumericString = 18,
  DerNodeType_PrintableString = 19,
  DerNodeType_T61String = 20,
  DerNodeType_VideoTexString = 21,
  DerNodeType_Ia5String = 22,
  DerNodeType_UtcTime = 23,
  DerNodeType_GeneralizedTime = 24,
  DerNodeType_GraphicString = 25,
  DerNodeType_VisibleString = 26,
  DerNodeType_GeneralString = 27,
  DerNodeType_UniversalString = 28,
  DerNodeType_CharacterString = 29,
  DerNodeType_BmpString = 30
};

}

#endif
