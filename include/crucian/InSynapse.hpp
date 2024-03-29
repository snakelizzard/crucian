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

#ifndef NTA_INSYNAPSE_HPP
#define NTA_INSYNAPSE_HPP

#include <fstream>
#include <ostream>

#include <crucian/Types.hpp>

//--------------------------------------------------------------------------------

namespace crucian
{

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
/**
 * The type of synapse contained in a Segment. It has the source cell index
 * of the synapse, and a permanence value. The source cell index is between
 * 0 and nCols * nCellsPerCol.
 */
class InSynapse
{
private:
    UInt _srcCellIdx;
    Real _permanence;

public:
    inline InSynapse() : _srcCellIdx((UInt)-1), _permanence(0) {}

    inline InSynapse(UInt srcCellIdx, Real permanence)
        : _srcCellIdx(srcCellIdx), _permanence(permanence)
    {
    }

    inline bool operator==(const InSynapse &other) const
    {
        return _srcCellIdx == other._srcCellIdx &&
               _permanence == other._permanence;
    }
    inline bool operator!=(const InSynapse &other) const
    {
        return !operator==(other);
    }

    inline UInt srcCellIdx() const { return _srcCellIdx; }
    const inline Real &permanence() const { return _permanence; }
    inline Real &permanence() { return _permanence; }

    inline void print(std::ostream &outStream) const;
};

//--------------------------------------------------------------------------------
#ifndef SWIG
std::ostream &operator<<(std::ostream &outStream, const InSynapse &s);
#endif

} // namespace crucian

#endif // NTA_INSYNAPSE_HPP
