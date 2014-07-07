/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_INTEREST_TYPES_H
#define NDN_INTEREST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ndn_Exclude_COMPONENT = 0,
  ndn_Exclude_ANY = 1
} ndn_ExcludeType;

enum {
  ndn_Interest_CHILD_SELECTOR_LEFT = 0,
  ndn_Interest_CHILD_SELECTOR_RIGHT = 1,

  ndn_Interest_ANSWER_NO_CONTENT_STORE = 0,
  ndn_Interest_ANSWER_CONTENT_STORE = 1,
  ndn_Interest_ANSWER_GENERATED = 2,
  ndn_Interest_ANSWER_STALE = 4,    // Stale answer OK
  ndn_Interest_MARK_STALE = 16,      // Must have scope 0.  Michael calls this a "hack"

  ndn_Interest_DEFAULT_ANSWER_ORIGIN_KIND = ndn_Interest_ANSWER_CONTENT_STORE | ndn_Interest_ANSWER_GENERATED
};

#ifdef __cplusplus
}
#endif

#endif
