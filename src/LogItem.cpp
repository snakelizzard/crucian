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

/** @file
 * LogItem implementation
 */

#include <iostream> // cerr

#include <crucian/Exception.hpp>
#include <crucian/LogItem.hpp>

namespace crucian
{

std::ostream *LogItem::ostream_ = nullptr;

void LogItem::setOutputFile(std::ostream &stream) { ostream_ = &stream; }

LogItem::LogItem(const char *filename, int line, LogLevel level)
    : filename_(filename), lineno_(line), level_(level), msg_(""),
      willThrow(false)
{
}

LogItem::~LogItem() noexcept(false)
{
    std::string level;
    switch (level_)
    {
    case debug:
        level = "DEBUG:";
        break;
    case warn:
        level = "WARN: ";
        break;
    case info:
        level = "INFO: ";
        break;
    case error:
        level = "ERR:";
        break;
        //  default:
        //    slevel = "Unknown: ";
        //    break;
    }

    if (ostream_ == nullptr)
        ostream_ = &(std::cerr);

    (*ostream_) << level << "  " << msg_.str();

    if (level_ == error)
        (*ostream_) << " [" << filename_ << " line " << lineno_ << "]";

    (*ostream_) << std::endl;

    if (willThrow)
        throw Exception(msg_.str());
}

std::ostringstream &LogItem::stream() { return msg_; }

std::ostringstream &LogItem::throwStream()
{
    willThrow = true;
    return msg_;
}

} // namespace crucian
