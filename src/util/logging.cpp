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

#include <ndn-cpp/util/logging.hpp>

#if NDN_CPP_HAVE_LOG4CXX

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/level.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/defaultconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

#include <unistd.h>

void
INIT_LOGGERS ()
{
  static bool configured = false;

  if (configured) return;

  if (access ("log4cxx.properties", R_OK)==0)
    PropertyConfigurator::configureAndWatch ("log4cxx.properties");
  else
    {
      PatternLayoutPtr   layout   (new PatternLayout ("%d{HH:mm:ss} %p %c{1} - %m%n"));
      ConsoleAppenderPtr appender (new ConsoleAppender (layout));

      BasicConfigurator::configure( appender );
      Logger::getRootLogger()->setLevel (log4cxx::Level::getInfo ());
    }

  configured = true;
}

#endif
