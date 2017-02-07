/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
 * @author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 * @author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
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

#ifndef NDN_LOGGING_HPP
#define NDN_LOGGING_HPP

#include "../common.hpp"

#if NDN_CPP_HAVE_LOG4CXX

#include <log4cxx/logger.h>

#define MEMBER_LOGGER                           \
  static log4cxx::LoggerPtr staticModuleLogger;

#define INIT_MEMBER_LOGGER(className,name)          \
  log4cxx::LoggerPtr className::staticModuleLogger =  log4cxx::Logger::getLogger (name);

#define INIT_LOGGER(name) \
  static log4cxx::LoggerPtr staticModuleLogger = log4cxx::Logger::getLogger (name);

#define _LOG_DEBUG(x) \
  LOG4CXX_DEBUG(staticModuleLogger, x);

#define _LOG_TRACE(x) \
  LOG4CXX_TRACE(staticModuleLogger, x);

#define _LOG_FUNCTION(x) \
  LOG4CXX_TRACE(staticModuleLogger, __FUNCTION__ << "(" << x << ")");

#define _LOG_FUNCTION_NOARGS \
  LOG4CXX_TRACE(staticModuleLogger, __FUNCTION__ << "()");

#define _LOG_ERROR(x) \
  LOG4CXX_ERROR(staticModuleLogger, x);

#define _LOG_ERROR_COND(cond,x) \
  if (cond) { _LOG_ERROR(x) }

#define _LOG_DEBUG_COND(cond,x) \
  if (cond) { _LOG_DEBUG(x) }

void
INIT_LOGGERS ();

#else // else NDN_CPP_HAVE_LOG4CXX

#define INIT_LOGGER(name)
#define _LOG_FUNCTION(x)
#define _LOG_FUNCTION_NOARGS
#define _LOG_TRACE(x)
#define INIT_LOGGERS(x)
#define _LOG_ERROR_COND(cond,x)
#define _LOG_DEBUG_COND(cond,x)

#define MEMBER_LOGGER
#define INIT_MEMBER_LOGGER(className,name)

#ifdef _DEBUG

#include <time.h>
#include <iostream>

#define _LOG_DEBUG(x) \
  { time_t now = time(0); std::string s = std::string(ctime(&now)); std::clog << s.substr(0, s.size() - 1) << " " << x << std::endl; }
#define _LOG_ERROR(x) _LOG_DEBUG(x)

#else
#define _LOG_DEBUG(x)
#define _LOG_ERROR(x)
#endif

#endif // NDN_CPP_HAVE_LOG4CXX

#endif
