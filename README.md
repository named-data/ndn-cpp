--------------------------------------------------------------
NDN-CPP:  A Named Data Networking client library for C++ and C
--------------------------------------------------------------

NDN-CPP is a new implementation of a Named Data Networking client library written in C++ and C.
It is wire format compatible with the NDN-TLV encoding.

NDN-CPP conforms to the NDN Common Client Libraries API and your application should
follow this public API:
http://named-data.net/doc/ndn-ccl-api/ .
Further classes and methods are in the Doxygen-generated documentation.

See the file [INSTALL.md](https://github.com/named-data/ndn-cpp/blob/master/INSTALL.md) for build and install instructions.

Please submit any bugs or issues to the NDN-CPP issue tracker:
http://redmine.named-data.net/projects/ndn-cpp/issues

---

The library currently requires a remote NDN daemon, and has been tested with the
NFD forwarder: https://github.com/named-data/NFD

Currently, the library has two APIs for developers:

	1. The C++ API which follows the NDN Common Client Library API also used by ndn-js (JavaScript)
     and PyNDN (Python). See http://named-data.net/doc/ndn-ccl-api .

	2. A core C library implementing lower-level encoding and communication.  Applications should normally
     use the C++ API, but core C code will also function on embedded devices and other environments
     which don't have C++ support.

License
-------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version, with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
A copy of the GNU Lesser General Public License is in the file COPYING.
