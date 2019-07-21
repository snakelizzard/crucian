/* ---------------------------------------------------------------------
 * Numenta Platform for Intelligent Computing (NuPIC)
 * Copyright (C) 2013, Numenta, Inc.  Unless you have an agreement
 * with Numenta, Inc., for a separate license for this software code, the
 * following terms and conditions apply:
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with this program.  If not, see http://www.gnu.org/licenses.
 *
 * http://numenta.org/licenses/
 * ---------------------------------------------------------------------
 */

/** @file */

//----------------------------------------------------------------------

#include "nust/utils/LoggingException.hpp"
#include "nust/utils/LogItem.hpp"
#include <iostream>
using namespace nust;

LoggingException::~LoggingException() {
  try {
    if (!alreadyLogged_) {
      // Let LogItem do the work for us. This code is a bit complex
      // because LogItem was designed to be used from a logging macro
      LogItem *li = new LogItem(filename_.c_str(), static_cast<int> (lineno_), LogItem::error);
      li->stream() << getMessage();
      delete li;

      alreadyLogged_ = true;
    }
  } catch(...) {
    // nothing
  }
}
