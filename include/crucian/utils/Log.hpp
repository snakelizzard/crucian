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

/**
 * @file
 * Definition of C++ macros for logging.
 */

#ifndef NTA_LOG2_HPP
#define NTA_LOG2_HPP

#include <crucian/utils/LogItem.hpp>

// For informational messages that report status but do not indicate that
// anything is wrong
#define NTA_INFO crucian::LogItem(__FILE__, __LINE__, crucian::LogItem::info).stream()

// To throw an exception and make sure the exception message is logged
// appropriately
#define NTA_THROW                                                              \
    crucian::LogItem(__FILE__, __LINE__, crucian::LogItem::error).throwStream()

// The difference between CHECK and ASSERT is that ASSERT is for
// performance critical code and can be disabled in a release
// build. Both throw an exception on error.

#define NTA_CHECK(condition)                                                   \
    if (condition)                                                             \
    {                                                                          \
    }                                                                          \
    else                                                                       \
        NTA_THROW << "CHECK FAILED: \"" << #condition << "\" "

#ifdef NTA_ASSERTIONS_ON

#define NTA_ASSERT(condition)                                                  \
    if (condition)                                                             \
    {                                                                          \
    }                                                                          \
    else                                                                       \
        NTA_THROW << "ASSERTION FAILED: \"" << #condition << "\" "

#else

// NTA_ASSERT macro does nothing.
// The second line should never be executed, or even compiled, but we
// need something that is syntactically compatible with NTA_ASSERT
#define NTA_ASSERT(condition)                                                  \
    if (1)                                                                     \
    {                                                                          \
    }                                                                          \
    else if (!(condition))                                                     \
    crucian::LogItem(__FILE__, __LINE__, crucian::LogItem::debug).stream()

#endif // NTA_ASSERTIONS_ON

#endif // NTA_LOG2_HPP
