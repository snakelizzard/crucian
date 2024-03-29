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
 * Definitions for various utility functions
 */

#ifndef NTA_UTILS_HPP
#define NTA_UTILS_HPP

#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <math.h>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <crucian/Log.hpp>
#include <crucian/Types.hpp>

namespace crucian
{

//--------------------------------------------------------------------------------
/**
 * Computes the amount of padding required to align two adjacent blocks of
 * memory. If the first block has 17 bytes, and the second is a "vector" of 4
 * elements of 4 bytes each, we need to align the start of the "vector" on a 4
 * bytes boundary. The amount of padding required after the 17 bytes of the
 * first block is: 3 bytes, and 3 = 4 - 17 % 4, that is: padding = second elem
 * size - first total size % second elem size.
 *
 * Special case: if the first block of memory ends on a boundary of the second
 * block, no padding required. Example, first block has 16 bytes and second
 * vector of 4 bytes each: 16 % 4 = 0.
 */
template <typename SizeType>
inline const SizeType padding(const SizeType &s1, const SizeType &s2)
{
    if (s2)
    {
        SizeType extra = s1 % s2;
        return extra == 0 ? 0 : s2 - extra;
    }
    else
        return 0;
}

/*
  the following code is known to cause -Wstrict-aliasing warning, so silence it
  here
*/
#if !defined(NTA_OS_WINDOWS)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

inline bool isSystemLittleEndian()
{
    static const char test[2] = {1, 0};
    return (*reinterpret_cast<const short *>(test)) == 1;
}

#if !defined(NTA_OS_WINDOWS)
#pragma GCC diagnostic pop // return back to defaults
#endif

template <typename T> inline void swapBytesInPlace(T *pxIn, Size n)
{
    union SwapType {
        T x;
        unsigned char b[sizeof(T)];
    };
    auto *px = reinterpret_cast<SwapType *>(pxIn);
    SwapType *pxend = px + n;
    const int stop = sizeof(T) / 2;
    for (; px != pxend; ++px)
    {
        for (int j = 0; j < stop; ++j)
            std::swap(px->b[j], px->b[sizeof(T) - j - 1]);
    }
}

template <typename T> inline void swapBytes(T *pxOut, Size n, const T *pxIn)
{
    NTA_ASSERT(pxOut != pxIn) << "Use swapBytesInPlace() instead.";
    NTA_ASSERT(!(((pxOut > pxIn) && (pxOut < (pxIn + n))) ||
                 ((pxIn > pxOut) && (pxIn < (pxOut + n)))))
        << "Overlapping ranges not supported.";

    union SwapType {
        T x;
        unsigned char b[sizeof(T)];
    };
    const SwapType *px0 = reinterpret_cast<SwapType *>(pxIn);
    const SwapType *pxend = px0 + n;
    auto *px1 = reinterpret_cast<SwapType *>(pxOut);
    for (; px0 != pxend; ++px0, ++px1)
    {
        for (int j = 0; j < sizeof(T); ++j)
            px1->b[j] = px0->b[sizeof(T) - j - 1];
    }
}

/**
 * Calculates sizeof() types named by string names of types in
 * nust/types/types. Throws if the requested type cannot be found.
 *
 * Supported type names include:
 * bool,
 * char, wchar_t,
 * NTA_Char, NTA_WChar, NTA_Byte,
 * float, double,
 * NTA_Real32, NTA_Real64, NTA_Real,
 * int, size_t,
 * NTA_Int32, NTA_UInt32, NTA_Int64, NTA_UInt64, NTA_Size
 *
 * @param name (string) Name of type to calculate sizeof() for.
 * @param isNumeric (bool&) set to true on exit if type name is a number.
 * @retval Number of bytes per element of the specified type.
 */
extern size_t GetTypeSize(const std::string &name, bool &isNumeric);

/**
 * Calculates sizeof() types named by string names of types in
 * nust/types/types. Throws if the requested type cannot be found.
 *
 * Supported type names include:
 * bool,
 * char, wchar_t,
 * NTA_Char, NTA_WChar, NTA_Byte,
 * float, double,
 * NTA_Real32, NTA_Real64, NTA_Real,
 * int, size_t,
 * NTA_Int32, NTA_UInt32, NTA_Int64, NTA_UInt64, NTA_Size
 *
 * @param name (string) Name of type to calculate sizeof() for.
 * @param isNumeric (bool&) set to true on exit if type name is a number.
 * @retval Number of bytes per element of the specified type.
 */
extern size_t GetTypeSize(NTA_BasicType type, bool &isNumeric);

/**
 * Return a string representation of an NTA_BasicType
 *
 * @param type the NTA_BasicType enum
 * @retval name of the type as a string
 */
extern std::string GetTypeName(NTA_BasicType type);

/**
 * Utility routine used by PrintVariableArray to print array of a certain type
 */
template <typename T>
inline void utilsPrintArray_(std::ostream &out, const void *theBeginP,
                             const void *theEndP)
{
    const T *beginP = static_cast<const T *>(theBeginP);
    const T *endP = static_cast<const T *>(theEndP);

    for (; beginP != endP; ++beginP)
        out << *beginP << " ";
}

/**
 * Utility routine for setting an array in memory of a certain type from a
 * stream
 *
 * @param in        the stream with values to put into the array
 * @param theBeginP pointer to start of array in memory
 * @param theEndP   pointer to end of array in memory
 * @retval          true if successfully set all values
 *
 */
template <typename T>
inline void utilsSetArray_(std::istream &in, void *theBeginP, void *theEndP)
{
    T *beginP = static_cast<T *>(theBeginP);
    T *endP = static_cast<T *>(theEndP);

    for (; beginP != endP && in.good(); ++beginP)
        in >> *beginP;
    if (beginP != endP && !in.eof())
        NTA_THROW << "UtilsSetArray() - error reading stream of values";
}

/**
 * Streams the contents of a variable array cast as the given type.
 *
 * This is used by the NodeProcessor when returing the value of an node's
 * outputs in response to the "nodeOPrint" supervisor command, and also when
 * returning the value of a node's output or parameters to the tools in response
 * to a watch request.
 *
 * The caller must pass in either a dataType, elemSize, or both. If both are
 * specified, then this routine will assert that the elemSize agrees with the
 * given dataType. If dataType is not specified, then this routine will pick a
 * most likely dataType given the elemSize.
 *
 * @param outStream [std::ostream] the stream to print to
 * @param beginP [Byte*] pointer to the start of the variable
 * @param endP   [Byte*] pointer to first byte past the end of the variable
 * @param dataType [std::string] the data type to print as (optional)
 * @retval [std::string] the actual type the variable was printed as. This will
 *         always be dataType, unless the dataType was unrecognized.
 *
 * @b Exceptions:
 *  @li None.
 */
extern std::string PrintVariableArray(std::ostream &outStream,
                                      const Byte *beginP, const Byte *endP,
                                      const std::string &dataType = "");

/**
 * Sets the contents of a variable array cast as the given type.
 *
 * This is used by the NodeProcessor when setting the value of an node's outputs
 * in response to the "nodeOSet" supervisor command.
 *
 * @param inStream [std::istream] the stream to fetch the values from
 * @param beginP [Byte*] pointer to the start of the variable
 * @param endP   [Byte*] pointer to first byte past the end of the variable
 * @param dataType [std::string] the data type to set as
 *
 * @b Exceptions:
 *  @li None.
 */
extern void SetVariableArray(std::istream &inStream, Byte *beginP, Byte *endP,
                             const std::string &dataType);

//--------------------------------------------------------------------------------
// Defines, used as code generators, to make the code more readable

/**
 * Function object that takes a single argument, a pair (or at least
 * a class with the same interface as pair), and returns the pair's
 * first element. This is not part of the C++ standard, but usually
 * provided by implementations of STL.
 */
template <class Pair>
struct select1st : public std::unary_function<Pair, typename Pair::first_type>
{
    inline const typename Pair::first_type &operator()(const Pair &x) const
    {
        return x.first;
    }
};

/**
 * Function object that takes a single argument, a pair (or at least
 * a class with the same interface as pair), and returns the pair's
 * second element. This is not part of the C++ standard, but usually
 * provided by implementations of STL.
 */
template <class Pair>
struct select2nd : public std::unary_function<Pair, typename Pair::second_type>
{
    inline const typename Pair::second_type &operator()(const Pair &x) const
    {
        return x.second;
    }
};

} // namespace crucian

#endif // NTA_UTILS_HPP
