/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
 * @author: From https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/util/config-file.hpp
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

#ifndef NDN_CONFIG_FILE_HPP
#define NDN_CONFIG_FILE_HPP

#include <string>
#include <map>
#include <exception>

namespace ndn {

/**
 * A ConfigFile locates, opens, and parses a library configuration file, and
 * holds the values for the application to get.
 */
class ConfigFile {
public:
  /**
   * Locate, open, and parse a library configuration file.
   */
  ConfigFile();

  class Error : public std::exception {
  public:
    Error(const std::string& errorMessage) throw()
    : errorMessage_(errorMessage)
    {
    }

    virtual ~Error() throw();

    std::string
    Msg() const { return errorMessage_; }

    virtual const char*
    what() const throw() { return errorMessage_.c_str(); }

  private:
    const std::string errorMessage_;
  };

  /**
   * Get the value for the key, or a default value if not found.
   * @param key The key to search for.
   * @param defaultValue The default value if the key is not found.
   * @return The value, or defaultValue if the key is not found.
   */
  std::string
  get(const std::string& key, const std::string& defaultValue) const
  {
    std::map<std::string, std::string>::const_iterator found = config_.find(key);
    if (found != config_.end())
      return found->second;
    else
      return defaultValue;
  }

  /**
   * Get the path of the configuration file.
   * @return The path or an empty string if not found.
   */
  const std::string&
  getPath() const { return path_; }

  /**
   * Get the configuration key/value pairs.
   * @return A map of key/value pairs.
   */
  const std::map<std::string, std::string>&
  getParsedConfiguration() const { return config_; }

private:
  /**
   * Look for the configuration file in these well-known locations:
   *
   * 1. $HOME/.ndn/client.conf
   * 2. @SYSCONFDIR@/ndn/client.conf
   * 3. /etc/ndn/client.conf
   *
   * @return The path of the config file or an empty string if not found.
   */
  static std::string
  findConfigFile();

  /**
   * Open path_, parse the configuration file and set config_.
   */
  void
  parse();

  std::string path_;
  std::map<std::string, std::string> config_;
};

}

#endif
