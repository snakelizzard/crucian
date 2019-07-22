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
 * A few types used in nust/math and nust/algorithms.
 */

#ifndef NTA_MATH_TYPES_HPP
#define NTA_MATH_TYPES_HPP

#include <algorithm> // sort
#include <limits>
#include <set>
#include <vector>

#include <nust/types/Types.hpp>
#include <nust/utils/Log.hpp>

/* This file is used by array_algo.hpp */

namespace nust
{

//--------------------------------------------------------------------------------
// BYTE VECTOR
//--------------------------------------------------------------------------------
/**
 * This is a good compromise between speed and memory for the use cases we have.
 * Going to a real vector of bits is slower when accessing the individual bits,
 * but this vector of bytes can still be fed to the SSE with good results.
 */
struct ByteVector : public std::vector<nust::Byte>
{
    inline explicit ByteVector(size_t n = 0)
        : std::vector<nust::Byte>(n, static_cast<nust::Byte>(0))
    {
    }

    /**
     * Use these two functions when converting with a vector of int or float
     * since the byte represenation of the elements in a byte vector is _not_
     * the same as the byte representation of ints and floats.
     */
    template <typename It>
    inline ByteVector(It begin, size_t n) : std::vector<nust::Byte>(n, 0)
    {
        for (size_t i = 0; i != this->size(); ++i)
            (*this)[i] = *begin++ != 0;
    }

    template <typename It> inline void toDense(It begin, It /*end*/)
    {
        for (size_t i = 0; i != this->size(); ++i)
            *begin++ = (*this)[i] != 0;
    }
};

//--------------------------------------------------------------------------------
// Buffer
//--------------------------------------------------------------------------------
/**
 * Allocated once, but only the first n positions are valid (std::vector does
 * that!) DON'T USE ANYMORE, but keeping it because a lot of code already
 * depends on it.
 */
template <typename T> struct Buffer : private std::vector<T>
{
    typedef size_t size_type;
    typedef T value_type;

    size_type nnz;

    inline explicit Buffer(size_type _s = 0) : std::vector<T>(_s), nnz(0) {}

    inline void buf_clear() { nnz = 0; }

    inline void adjust_nnz(size_t n) // call resize?
    {
        nnz = std::min(nnz, n);
    }

    inline bool buf_empty() const { return nnz == 0; }

    inline void buf_push_back(const T &x) { (*this)[nnz++] = x; }

    inline typename std::vector<T>::iterator nnz_end()
    {
        return this->begin() + nnz;
    }

    inline typename std::vector<T>::const_iterator nnz_end() const
    {
        return this->begin() + nnz;
    }
};

//--------------------------------------------------------------------------------
/**
 * The first element of each pair is the index of a non-zero, and the second
 * element is the value of the non-zero.
 */
template <typename T1, typename T2>
struct SparseVector : public Buffer<std::pair<T1, T2>>
{
    typedef T1 size_type;
    typedef T2 value_type;

    inline explicit SparseVector(size_type s = 0) : Buffer<std::pair<T1, T2>>(s) {}
};

//--------------------------------------------------------------------------------
} // end namespace nust

#endif // NTA_MATH_TYPES_HPP
