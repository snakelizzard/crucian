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
 * Basic C++ type definitions used throughout `nust.core` and rely on `Types.h`
 */

#ifndef NTA_TYPES_HPP
#define NTA_TYPES_HPP

#include <nust/types/NtaTypes.hpp>

//----------------------------------------------------------------------

namespace nust
{
/**
 * @name Basic types
 *
 * @{
 */

/**
 * Represents a 8-bit byte.
 */
typedef NTA_Byte Byte;

/**
 * Represents a 16-bit signed integer.
 */
typedef NTA_Int16 Int16;

/**
 * Represents a 16-bit unsigned integer.
 */
typedef NTA_UInt16 UInt16;

/**
 * Represents a 32-bit signed integer.
 */
typedef NTA_Int32 Int32;

/**
 * Represents a 32-bit unsigned integer.
 */
typedef NTA_UInt32 UInt32;

/**
 * Represents a 64-bit signed integer.
 */
typedef NTA_Int64 Int64;

/**
 * Represents a 64-bit unsigned integer.
 */
typedef NTA_UInt64 UInt64;

/**
 * Represents a 32-bit real number(a floating-point number).
 */
typedef NTA_Real32 Real32;

/**
 * Represents a 64-bit real number(a floating-point number).
 */
typedef NTA_Real64 Real64;

/**
 * Represents an opaque handle/pointer, same as `void *`
 */
typedef NTA_Handle Handle;

/**
 * Represents an opaque pointer, same as `uintptr_t`
 */
typedef NTA_UIntPtr UIntPtr;

/**
 * @}
 */

/**
 * @name Flexible types
 *
 * The following are flexible types depending on `NTA_DOUBLE_PRECISION` and
 * `NTA_BIG_INTEGER`.
 *
 * @{
 *
 */

/**
 * Represents a real number(a floating-point number).
 *
 * Same as nust::Real64 if `NTA_DOUBLE_PRECISION` is defined, nust::Real32
 * otherwise.
 */
typedef NTA_Real Real;

/**
 * Represents a signed integer.
 *
 * Same as nust::Int64 if `NTA_BIG_INTEGER` is defined, nust::Int32 otherwise.
 */
typedef NTA_Int Int;

/**
 * Represents a unsigned integer.
 *
 * Same as nust::UInt64 if `NTA_BIG_INTEGER` is defined, nust::UInt32
 * otherwise.
 */
typedef NTA_UInt UInt;

/**
 * Represents lengths of arrays, strings and so on.
 */
typedef NTA_Size Size;

/**
 * @}
 */

} // end namespace nust

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define NUST_DLL_IMPORT __declspec(dllimport)
  #define NUST_DLL_EXPORT __declspec(dllexport)
  #define NUST_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define NUST_DLL_IMPORT __attribute__ ((visibility ("default")))
#define NUST_DLL_EXPORT __attribute__ ((visibility ("default")))
#define NUST_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define NUST_DLL_IMPORT
    #define NUST_DLL_EXPORT
    #define NUST_DLL_LOCAL
#endif
#endif

#ifdef NUST_DLL
#ifdef NUST_DLL_EXPORTS
#define NUST_API NUST_DLL_EXPORT
#else
#define NUST_API NUST_DLL_IMPORT
#endif // NUST_DLL_EXPORTS
#define NUST_LOCAL NUST_DLL_LOCAL
#else // NUST_DLL
#define NUST_API
#define NUST_LOCAL
#endif // NUST_DLL

#endif // NTA_TYPES_HPP
