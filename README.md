NDN-CPP:  A Named Data Networking client library for C++ and C
--------------------------------------------------------------

NDN-CPP is a new implementation of a Named Data Networking client library written in C++ and C.  
It is wire format compatible with the new NDN-TLV encoding, with NDNx and PARC's CCNx.
	
See the file INSTALL.md for build and install instructions.

Please submit any bugs or issues to the NDN-CPP issue tracker:
http://redmine.named-data.net/projects/ndn-cpp/issues

---
	
The library currently requires a remote NDN daemon, and has been tested with ndnd, from
the NDNx package: https://github.com/named-data/ndnx .

Currently, the library has two APIs for developers: 

	1. The C++ API which follows the NDN Common Client Library API also used by ndn-js (JavaScript)
     and PyNDN (Python). See http://named-data.net/doc/ndn-ccl-api .

	2. A core C library implementing lower-level encoding and communication.  Applications should normally
     use the C++ API, but core C code will also function on embedded devices and other environments 
     which don't have C++ support.

License
-------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version, with the additional exemption that 
compiling, linking, and/or using OpenSSL is allowed.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
A copy of the GNU General Public License is in the file COPYING.
