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

#include <crucian/NtaTypes.hpp>

//----------------------------------------------------------------------

namespace crucian
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
 * Same as crucian::Real64 if `NTA_DOUBLE_PRECISION` is defined, crucian::Real32
 * otherwise.
 */
typedef NTA_Real Real;

/**
 * Represents a signed integer.
 *
 * Same as crucian::Int64 if `NTA_BIG_INTEGER` is defined, crucian::Int32 otherwise.
 */
typedef NTA_Int Int;

/**
 * Represents a unsigned integer.
 *
 * Same as crucian::UInt64 if `NTA_BIG_INTEGER` is defined, crucian::UInt32
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

} // end namespace crucian

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define CRU_DLL_IMPORT __declspec(dllimport)
  #define CRU_DLL_EXPORT __declspec(dllexport)
  #define CRU_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define CRU_DLL_IMPORT __attribute__ ((visibility ("default")))
#define CRU_DLL_EXPORT __attribute__ ((visibility ("default")))
#define CRU_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define CRU_DLL_IMPORT
    #define CRU_DLL_EXPORT
    #define CRU_DLL_LOCAL
#endif
#endif

#ifdef CRU_DLL
#ifdef CRU_DLL_EXPORTS
#define CRU_API CRU_DLL_EXPORT
#else
#define CRU_API CRU_DLL_IMPORT
#endif // CRU_DLL_EXPORTS
#define CRU_LOCAL CRU_DLL_LOCAL
#else // CRU_DLL
#define CRU_API
#define CRU_LOCAL
#endif // CRU_DLL

#define CRU_UNUSED(x) (void)x

#endif // NTA_TYPES_HPP
