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
 * LogItem interface
 */

#ifndef NTA_LOG_ITEM_HPP
#define NTA_LOG_ITEM_HPP

#include <iostream>
#include <sstream>

#include <crucian/Exception.hpp>

namespace crucian
{

/**
 * @b Description
 * A LogItem represents a single log entry. It contains a stream that
 * accumulates a log message, and its destructor calls the logger.
 *
 * A LogItem contains an internal stream
 * which is used for building up an application message using
 * << operators.
 *
 */

class LogItem
{
public:
    typedef enum
    {
        debug,
        info,
        warn,
        error
    } LogLevel;

    /**
     * Record information to be logged
     */
    LogItem(const char *filename, int line, LogLevel level);

    /**
     * Destructor performs the logging
     */
    ~LogItem() noexcept(false);

    /*
     * Return the underlying stream object. Caller will use it to construct the
     * log message.
     */
    std::ostringstream &stream();

    std::ostringstream &throwStream();

    static void setOutputFile(std::ostream &stream);

protected:
    const char *filename_; // name of file
    int lineno_;           // line number in file
    LogLevel level_;
    std::ostringstream msg_;
    bool willThrow;

private:
    static std::ostream *ostream_;
};

} // namespace crucian

#endif // NTA_LOG_ITEM_HPP
