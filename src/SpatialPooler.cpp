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
 * ----------------------------------------------------------------------
 */

/** @file
 * Implementation of SpatialPooler
 */

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <crucian/Math.hpp>
#include <crucian/SpatialPooler.hpp>
#include <crucian/Topology.hpp>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace crucian
{

static const Real PERMANENCE_EPSILON = 0.000001;

// MSVC doesn't provide round() which only became standard in C99 or C++11
#if defined(NTA_COMPILER_MSVC)
template <typename T> T round(T num)
{
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}
#endif

// Round f to 5 digits of precision. This is used to set
// permanence values and help avoid small amounts of drift between
// platforms/implementations
static Real round5_(const Real f)
{
    Real p = ((Real)((Int)(f * 100000))) / 100000.0;
    return p;
}

class CoordinateConverter2D
{
public:
    CoordinateConverter2D(UInt nrows, UInt ncols)
        : // TODO param nrows is unused
          ncols_(ncols)
    {
        CRU_UNUSED(nrows);
    }
    UInt toRow(UInt index) { return index / ncols_; };
    UInt toCol(UInt index) { return index % ncols_; };

private:
    UInt ncols_;
};

class CoordinateConverterND
{
public:
    explicit CoordinateConverterND(std::vector<UInt> &dimensions)
    {
        dimensions_ = dimensions;
        UInt b = 1;
        for (Int i = (Int)dimensions.size() - 1; i >= 0; i--)
        {
            bounds_.insert(bounds_.begin(), b);
            b *= dimensions[i];
        }
    }

    void toCoord(UInt index, std::vector<UInt> &coord)
    {
        coord.clear();
        for (size_t i = 0; i < bounds_.size(); i++)
        {
            coord.push_back((index / bounds_[i]) % dimensions_[i]);
        }
    };

    UInt toIndex(std::vector<UInt> &coord)
    {
        UInt index = 0;
        for (size_t i = 0; i < coord.size(); i++)
        {
            index += coord[i] * bounds_[i];
        }
        return index;
    };

private:
    std::vector<UInt> dimensions_;
    std::vector<UInt> bounds_;
};

SpatialPooler::SpatialPooler()
{
    // The current version number.
    version_ = 2;
}

SpatialPooler::SpatialPooler(const std::vector<UInt> &inputDimensions,
                             const std::vector<UInt> &columnDimensions,
                             UInt potentialRadius, Real potentialPct,
                             bool globalInhibition, Real localAreaDensity,
                             UInt numActiveColumnsPerInhArea,
                             UInt stimulusThreshold, Real synPermInactiveDec,
                             Real synPermActiveInc, Real synPermConnected,
                             Real minPctOverlapDutyCycles, UInt dutyCyclePeriod,
                             Real boostStrength, Int seed, UInt spVerbosity,
                             bool wrapAround)
    : SpatialPooler::SpatialPooler()
{
    initialize(inputDimensions, columnDimensions, potentialRadius, potentialPct,
               globalInhibition, localAreaDensity, numActiveColumnsPerInhArea,
               stimulusThreshold, synPermInactiveDec, synPermActiveInc,
               synPermConnected, minPctOverlapDutyCycles, dutyCyclePeriod,
               boostStrength, seed, spVerbosity, wrapAround);
}

std::vector<UInt> SpatialPooler::getColumnDimensions() const
{
    return columnDimensions_;
}

std::vector<UInt> SpatialPooler::getInputDimensions() const
{
    return inputDimensions_;
}

UInt SpatialPooler::getNumColumns() const { return numColumns_; }

UInt SpatialPooler::getNumInputs() const { return numInputs_; }

UInt SpatialPooler::getPotentialRadius() const { return potentialRadius_; }

void SpatialPooler::setPotentialRadius(UInt potentialRadius)
{
    potentialRadius_ = potentialRadius;
}

Real SpatialPooler::getPotentialPct() const { return potentialPct_; }

void SpatialPooler::setPotentialPct(Real potentialPct)
{
    potentialPct_ = potentialPct;
}

bool SpatialPooler::getGlobalInhibition() const { return globalInhibition_; }

void SpatialPooler::setGlobalInhibition(bool globalInhibition)
{
    globalInhibition_ = globalInhibition;
}

Int SpatialPooler::getNumActiveColumnsPerInhArea() const
{
    return numActiveColumnsPerInhArea_;
}

void SpatialPooler::setNumActiveColumnsPerInhArea(
    UInt numActiveColumnsPerInhArea)
{
    NTA_ASSERT(numActiveColumnsPerInhArea > 0);
    numActiveColumnsPerInhArea_ = numActiveColumnsPerInhArea;
    localAreaDensity_ = 0;
}

Real SpatialPooler::getLocalAreaDensity() const { return localAreaDensity_; }

void SpatialPooler::setLocalAreaDensity(Real localAreaDensity)
{
    NTA_ASSERT(localAreaDensity > 0 && localAreaDensity <= 1);
    localAreaDensity_ = localAreaDensity;
    numActiveColumnsPerInhArea_ = 0;
}

UInt SpatialPooler::getStimulusThreshold() const { return stimulusThreshold_; }

void SpatialPooler::setStimulusThreshold(UInt stimulusThreshold)
{
    stimulusThreshold_ = stimulusThreshold;
}

UInt SpatialPooler::getInhibitionRadius() const { return inhibitionRadius_; }

void SpatialPooler::setInhibitionRadius(UInt inhibitionRadius)
{
    inhibitionRadius_ = inhibitionRadius;
}

UInt SpatialPooler::getDutyCyclePeriod() const { return dutyCyclePeriod_; }

void SpatialPooler::setDutyCyclePeriod(UInt dutyCyclePeriod)
{
    dutyCyclePeriod_ = dutyCyclePeriod;
}

Real SpatialPooler::getBoostStrength() const { return boostStrength_; }

void SpatialPooler::setBoostStrength(Real boostStrength)
{
    boostStrength_ = boostStrength;
}

UInt SpatialPooler::getIterationNum() const { return iterationNum_; }

void SpatialPooler::setIterationNum(UInt iterationNum)
{
    iterationNum_ = iterationNum;
}

UInt SpatialPooler::getIterationLearnNum() const { return iterationLearnNum_; }

void SpatialPooler::setIterationLearnNum(UInt iterationLearnNum)
{
    iterationLearnNum_ = iterationLearnNum;
}

UInt SpatialPooler::getSpVerbosity() const { return spVerbosity_; }

void SpatialPooler::setSpVerbosity(UInt spVerbosity)
{
    spVerbosity_ = spVerbosity;
}

bool SpatialPooler::getWrapAround() const { return wrapAround_; }

void SpatialPooler::setWrapAround(bool wrapAround) { wrapAround_ = wrapAround; }

UInt SpatialPooler::getUpdatePeriod() const { return updatePeriod_; }

void SpatialPooler::setUpdatePeriod(UInt updatePeriod)
{
    updatePeriod_ = updatePeriod;
}

Real SpatialPooler::getSynPermTrimThreshold() const
{
    return synPermTrimThreshold_;
}

void SpatialPooler::setSynPermTrimThreshold(Real synPermTrimThreshold)
{
    synPermTrimThreshold_ = synPermTrimThreshold;
}

Real SpatialPooler::getSynPermActiveInc() const { return synPermActiveInc_; }

void SpatialPooler::setSynPermActiveInc(Real synPermActiveInc)
{
    synPermActiveInc_ = synPermActiveInc;
}

Real SpatialPooler::getSynPermInactiveDec() const
{
    return synPermInactiveDec_;
}

void SpatialPooler::setSynPermInactiveDec(Real synPermInactiveDec)
{
    synPermInactiveDec_ = synPermInactiveDec;
}

Real SpatialPooler::getSynPermBelowStimulusInc() const
{
    return synPermBelowStimulusInc_;
}

void SpatialPooler::setSynPermBelowStimulusInc(Real synPermBelowStimulusInc)
{
    synPermBelowStimulusInc_ = synPermBelowStimulusInc;
}

Real SpatialPooler::getSynPermConnected() const { return synPermConnected_; }

void SpatialPooler::setSynPermConnected(Real synPermConnected)
{
    synPermConnected_ = synPermConnected;
}

Real SpatialPooler::getSynPermMax() const { return synPermMax_; }

void SpatialPooler::setSynPermMax(Real synPermMax) { synPermMax_ = synPermMax; }

Real SpatialPooler::getMinPctOverlapDutyCycles() const
{
    return minPctOverlapDutyCycles_;
}

void SpatialPooler::setMinPctOverlapDutyCycles(Real minPctOverlapDutyCycles)
{
    minPctOverlapDutyCycles_ = minPctOverlapDutyCycles;
}

void SpatialPooler::getBoostFactors(Real boostFactors[]) const
{
    copy(boostFactors_.begin(), boostFactors_.end(), boostFactors);
}

void SpatialPooler::setBoostFactors(Real boostFactors[])
{
    boostFactors_.assign(&boostFactors[0], &boostFactors[numColumns_]);
}

void SpatialPooler::getOverlapDutyCycles(Real overlapDutyCycles[]) const
{
    copy(overlapDutyCycles_.begin(), overlapDutyCycles_.end(),
         overlapDutyCycles);
}

void SpatialPooler::setOverlapDutyCycles(Real overlapDutyCycles[])
{
    overlapDutyCycles_.assign(&overlapDutyCycles[0],
                              &overlapDutyCycles[numColumns_]);
}

void SpatialPooler::getActiveDutyCycles(Real activeDutyCycles[]) const
{
    copy(activeDutyCycles_.begin(), activeDutyCycles_.end(), activeDutyCycles);
}

void SpatialPooler::setActiveDutyCycles(Real activeDutyCycles[])
{
    activeDutyCycles_.assign(&activeDutyCycles[0],
                             &activeDutyCycles[numColumns_]);
}

void SpatialPooler::getMinOverlapDutyCycles(Real minOverlapDutyCycles[]) const
{
    copy(minOverlapDutyCycles_.begin(), minOverlapDutyCycles_.end(),
         minOverlapDutyCycles);
}

void SpatialPooler::setMinOverlapDutyCycles(Real minOverlapDutyCycles[])
{
    minOverlapDutyCycles_.assign(&minOverlapDutyCycles[0],
                                 &minOverlapDutyCycles[numColumns_]);
}

void SpatialPooler::getPotential(UInt column, UInt potential[]) const
{
    NTA_ASSERT(column < numColumns_);
    potentialPools_.getRow(column, &potential[0], &potential[numInputs_]);
}

void SpatialPooler::setPotential(UInt column, UInt potential[])
{
    NTA_ASSERT(column < numColumns_);
    potentialPools_.rowFromDense(column, &potential[0], &potential[numInputs_]);
}

void SpatialPooler::getPermanence(UInt column, Real permanences[]) const
{
    NTA_ASSERT(column < numColumns_);
    permanences_.getRowToDense(column, permanences);
}

void SpatialPooler::setPermanence(UInt column, Real permanences[])
{
    NTA_ASSERT(column < numColumns_);
    std::vector<Real> perm;
    perm.assign(&permanences[0], &permanences[numInputs_]);
    updatePermanencesForColumn_(perm, column, false);
}

void SpatialPooler::getConnectedSynapses(UInt column,
                                         UInt connectedSynapses[]) const
{
    NTA_ASSERT(column < numColumns_);
    connectedSynapses_.getRow(column, &connectedSynapses[0],
                              &connectedSynapses[numInputs_]);
}

void SpatialPooler::getConnectedCounts(UInt connectedCounts[]) const
{
    copy(connectedCounts_.begin(), connectedCounts_.end(), connectedCounts);
}

const std::vector<UInt> &SpatialPooler::getOverlaps() const
{
    return overlaps_;
}

const std::vector<Real> &SpatialPooler::getBoostedOverlaps() const
{
    return boostedOverlaps_;
}

void SpatialPooler::initialize(const std::vector<UInt> &inputDimensions,
                               const std::vector<UInt> &columnDimensions,
                               UInt potentialRadius, Real potentialPct,
                               bool globalInhibition, Real localAreaDensity,
                               UInt numActiveColumnsPerInhArea,
                               UInt stimulusThreshold, Real synPermInactiveDec,
                               Real synPermActiveInc, Real synPermConnected,
                               Real minPctOverlapDutyCycles,
                               UInt dutyCyclePeriod, Real boostStrength,
                               Int seed, UInt spVerbosity, bool wrapAround)
{

    numInputs_ = 1;
    inputDimensions_.clear();
    for (auto &inputDimension : inputDimensions)
    {
        numInputs_ *= inputDimension;
        inputDimensions_.push_back(inputDimension);
    }

    numColumns_ = 1;
    columnDimensions_.clear();
    for (auto &columnDimension : columnDimensions)
    {
        numColumns_ *= columnDimension;
        columnDimensions_.push_back(columnDimension);
    }

    NTA_ASSERT(numColumns_ > 0);
    NTA_ASSERT(numInputs_ > 0);
    NTA_ASSERT(inputDimensions_.size() == columnDimensions_.size());
    NTA_ASSERT(numActiveColumnsPerInhArea > 0 ||
               (localAreaDensity > 0 && localAreaDensity <= 0.5));
    NTA_ASSERT(potentialPct > 0 && potentialPct <= 1);

    seed_((UInt64)(seed < 0 ? random() : seed));

    potentialRadius_ =
        potentialRadius > numInputs_ ? numInputs_ : potentialRadius;
    potentialPct_ = potentialPct;
    globalInhibition_ = globalInhibition;
    numActiveColumnsPerInhArea_ = numActiveColumnsPerInhArea;
    localAreaDensity_ = localAreaDensity;
    stimulusThreshold_ = stimulusThreshold;
    synPermInactiveDec_ = synPermInactiveDec;
    synPermActiveInc_ = synPermActiveInc;
    synPermBelowStimulusInc_ = synPermConnected / 10.0;
    synPermConnected_ = synPermConnected;
    minPctOverlapDutyCycles_ = minPctOverlapDutyCycles;
    dutyCyclePeriod_ = dutyCyclePeriod;
    boostStrength_ = boostStrength;
    spVerbosity_ = spVerbosity;
    wrapAround_ = wrapAround;
    synPermMin_ = 0.0;
    synPermMax_ = 1.0;
    synPermTrimThreshold_ = synPermActiveInc / 2.0;
    NTA_ASSERT(synPermTrimThreshold_ < synPermConnected_);
    updatePeriod_ = 50;
    initConnectedPct_ = 0.5;
    iterationNum_ = 0;
    iterationLearnNum_ = 0;

    tieBreaker_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        tieBreaker_[i] = 0.01 * rng_.getReal64();
    }

    potentialPools_.resize(numColumns_, numInputs_);
    permanences_.resize(numColumns_, numInputs_);
    connectedSynapses_.resize(numColumns_, numInputs_);
    connectedCounts_.resize(numColumns_);

    overlapDutyCycles_.assign(numColumns_, 0);
    activeDutyCycles_.assign(numColumns_, 0);
    minOverlapDutyCycles_.assign(numColumns_, 0.0);
    boostFactors_.assign(numColumns_, 1);
    overlaps_.resize(numColumns_);
    overlapsPct_.resize(numColumns_);
    boostedOverlaps_.resize(numColumns_);

    inhibitionRadius_ = 0;

    for (UInt i = 0; i < numColumns_; ++i)
    {
        std::vector<UInt> potential = mapPotential_(i, wrapAround_);
        std::vector<Real> perm = initPermanence_(potential, initConnectedPct_);
        potentialPools_.rowFromDense(i, potential.begin(), potential.end());
        updatePermanencesForColumn_(perm, i, true);
    }

    updateInhibitionRadius_();

    if (spVerbosity_ > 0)
    {
        printParameters();
        std::cout << "CPP SP seed                 = " << seed << std::endl;
    }
}

void SpatialPooler::compute(UInt inputArray[], bool learn, UInt activeArray[])
{
    updateBookeepingVars_(learn);
    calculateOverlap_(inputArray, overlaps_);
    calculateOverlapPct_(overlaps_, overlapsPct_);

    if (learn)
    {
        boostOverlaps_(overlaps_, boostedOverlaps_);
    }
    else
    {
        boostedOverlaps_.assign(overlaps_.begin(), overlaps_.end());
    }

    inhibitColumns_(boostedOverlaps_, activeColumns_);
    toDense_(activeColumns_, activeArray, numColumns_);

    if (learn)
    {
        adaptSynapses_(inputArray, activeColumns_);
        updateDutyCycles_(overlaps_, activeArray);
        bumpUpWeakColumns_();
        updateBoostFactors_();
        if (isUpdateRound_())
        {
            updateInhibitionRadius_();
            updateMinDutyCycles_();
        }
    }
}

void SpatialPooler::stripUnlearnedColumns(UInt activeArray[]) const
{
    for (UInt i = 0; i < numColumns_; i++)
    {
        if (activeDutyCycles_[i] == 0)
        {
            activeArray[i] = 0;
        }
    }
}

void SpatialPooler::toDense_(std::vector<UInt> &sparse, UInt dense[], UInt n)
{
    std::fill(dense, dense + n, 0);
    for (auto &elem : sparse)
    {
        UInt index = elem;
        dense[index] = 1;
    }
}

void SpatialPooler::boostOverlaps_(std::vector<UInt> &overlaps,
                                   std::vector<Real> &boosted)
{
    for (size_t i = 0; i < numColumns_; i++)
    {
        boosted[i] = overlaps[i] * boostFactors_[i];
    }
}

UInt SpatialPooler::mapColumn_(UInt column)
{
    std::vector<UInt> columnCoords;
    CoordinateConverterND columnConv(columnDimensions_);
    columnConv.toCoord(column, columnCoords);

    std::vector<UInt> inputCoords;
    inputCoords.reserve(columnCoords.size());
    for (size_t i = 0; i < columnCoords.size(); i++)
    {
        const Real inputCoord =
            ((Real)columnCoords[i] + 0.5) *
            (inputDimensions_[i] / (Real)columnDimensions_[i]);

        inputCoords.push_back(floor(inputCoord));
    }

    CoordinateConverterND inputConv(inputDimensions_);
    return inputConv.toIndex(inputCoords);
}

std::vector<UInt> SpatialPooler::mapPotential_(UInt column, bool wrapAround)
{
    const UInt centerInput = mapColumn_(column);

    std::vector<UInt> columnInputs;
    if (wrapAround)
    {
        for (UInt input : WrappingNeighborhood(centerInput, potentialRadius_,
                                               inputDimensions_))
        {
            columnInputs.push_back(input);
        }
    }
    else
    {
        for (UInt input :
             Neighborhood(centerInput, potentialRadius_, inputDimensions_))
        {
            columnInputs.push_back(input);
        }
    }

    UInt numPotential = round(columnInputs.size() * potentialPct_);

    std::vector<UInt> selectedInputs(numPotential, 0);
    rng_.sample(&columnInputs.front(), columnInputs.size(),
                &selectedInputs.front(), numPotential);

    std::vector<UInt> potential(numInputs_, 0);
    for (UInt input : selectedInputs)
    {
        potential[input] = 1;
    }

    return potential;
}

Real SpatialPooler::initPermConnected_()
{
    Real p = synPermConnected_ +
             (synPermMax_ - synPermConnected_) * rng_.getReal64();

    return round5_(p);
}

Real SpatialPooler::initPermNonConnected_()
{
    Real p = synPermConnected_ * rng_.getReal64();
    return round5_(p);
}

std::vector<Real> SpatialPooler::initPermanence_(std::vector<UInt> &potential,
                                                 Real connectedPct)
{
    std::vector<Real> perm(numInputs_, 0);
    for (UInt i = 0; i < numInputs_; i++)
    {
        if (potential[i] < 1)
        {
            continue;
        }

        if (rng_.getReal64() <= connectedPct)
        {
            perm[i] = initPermConnected_();
        }
        else
        {
            perm[i] = initPermNonConnected_();
        }
        perm[i] = perm[i] < synPermTrimThreshold_ ? 0 : perm[i];
    }

    return perm;
}

void SpatialPooler::clip_(std::vector<Real> &perm, bool trim = false)
{
    Real minVal = trim ? synPermTrimThreshold_ : synPermMin_;
    for (auto &elem : perm)
    {
        elem = elem > synPermMax_ ? synPermMax_ : elem;
        elem = elem < minVal ? synPermMin_ : elem;
    }
}

void SpatialPooler::updatePermanencesForColumn_(std::vector<Real> &perm,
                                                UInt column, bool raisePerm)
{
    std::vector<UInt> connectedSparse;

    UInt numConnected;
    if (raisePerm)
    {
        std::vector<UInt> potential;
        potential.resize(numInputs_);
        potential = potentialPools_.getSparseRow(column);
        raisePermanencesToThreshold_(perm, potential);
    }

    numConnected = 0;
    for (size_t i = 0; i < perm.size(); ++i)
    {
        if (perm[i] >= synPermConnected_ - PERMANENCE_EPSILON)
        {
            connectedSparse.push_back(i);
            ++numConnected;
        }
    }

    clip_(perm, true);
    connectedSynapses_.replaceSparseRow(column, connectedSparse.begin(),
                                        connectedSparse.end());
    permanences_.setRowFromDense(column, perm);
    connectedCounts_[column] = numConnected;
}

UInt SpatialPooler::countConnected_(std::vector<Real> &perm)
{
    UInt numConnected = 0;
    for (auto &elem : perm)
    {
        if (elem >= synPermConnected_ - PERMANENCE_EPSILON)
        {
            ++numConnected;
        }
    }
    return numConnected;
}

UInt SpatialPooler::raisePermanencesToThreshold_(std::vector<Real> &perm,
                                                 std::vector<UInt> &potential)
{
    clip_(perm, false);
    UInt numConnected;
    while (true)
    {
        numConnected = countConnected_(perm);
        if (numConnected >= stimulusThreshold_)
            break;

        for (auto &elem : potential)
        {
            UInt index = elem;
            perm[index] += synPermBelowStimulusInc_;
        }
    }
    return numConnected;
}

void SpatialPooler::updateInhibitionRadius_()
{
    if (globalInhibition_)
    {
        inhibitionRadius_ =
            *max_element(columnDimensions_.begin(), columnDimensions_.end());
        return;
    }

    Real connectedSpan = 0;
    for (UInt i = 0; i < numColumns_; i++)
    {
        connectedSpan += avgConnectedSpanForColumnND_(i);
    }
    connectedSpan /= numColumns_;
    Real columnsPerInput = avgColumnsPerInput_();
    Real diameter = connectedSpan * columnsPerInput;
    Real radius = (diameter - 1) / 2.0;
    radius = std::max((Real)1.0, radius);
    inhibitionRadius_ = UInt(round(radius));
}

void SpatialPooler::updateMinDutyCycles_()
{
    if (globalInhibition_ ||
        inhibitionRadius_ >
            *max_element(columnDimensions_.begin(), columnDimensions_.end()))
    {
        updateMinDutyCyclesGlobal_();
    }
    else
    {
        updateMinDutyCyclesLocal_();
    }
}

void SpatialPooler::updateMinDutyCyclesGlobal_()
{
    Real maxOverlapDutyCycles =
        *max_element(overlapDutyCycles_.begin(), overlapDutyCycles_.end());

    fill(minOverlapDutyCycles_.begin(), minOverlapDutyCycles_.end(),
         minPctOverlapDutyCycles_ * maxOverlapDutyCycles);
}

void SpatialPooler::updateMinDutyCyclesLocal_()
{
    for (UInt i = 0; i < numColumns_; i++)
    {
        Real maxActiveDuty = 0;
        Real maxOverlapDuty = 0;
        if (wrapAround_)
        {
            for (UInt column :
                 WrappingNeighborhood(i, inhibitionRadius_, columnDimensions_))
            {
                maxActiveDuty =
                    std::max(maxActiveDuty, activeDutyCycles_[column]);
                maxOverlapDuty =
                    std::max(maxOverlapDuty, overlapDutyCycles_[column]);
            }
        }
        else
        {
            for (UInt column :
                 Neighborhood(i, inhibitionRadius_, columnDimensions_))
            {
                maxActiveDuty =
                    std::max(maxActiveDuty, activeDutyCycles_[column]);
                maxOverlapDuty =
                    std::max(maxOverlapDuty, overlapDutyCycles_[column]);
            }
        }

        minOverlapDutyCycles_[i] = maxOverlapDuty * minPctOverlapDutyCycles_;
    }
}

void SpatialPooler::updateDutyCycles_(std::vector<UInt> &overlaps,
                                      const UInt activeArray[])
{
    std::vector<UInt> newOverlapVal(numColumns_, 0);
    std::vector<UInt> newActiveVal(numColumns_, 0);

    for (UInt i = 0; i < numColumns_; i++)
    {
        newOverlapVal[i] = overlaps[i] > 0 ? 1 : 0;
        newActiveVal[i] = activeArray[i] > 0 ? 1 : 0;
    }

    UInt period =
        dutyCyclePeriod_ > iterationNum_ ? iterationNum_ : dutyCyclePeriod_;

    updateDutyCyclesHelper_(overlapDutyCycles_, newOverlapVal, period);
    updateDutyCyclesHelper_(activeDutyCycles_, newActiveVal, period);
}

Real SpatialPooler::avgColumnsPerInput_()
{
    UInt numDim = std::max(columnDimensions_.size(), inputDimensions_.size());
    Real columnsPerInput = 0;
    for (UInt i = 0; i < numDim; i++)
    {
        Real col = (i < columnDimensions_.size()) ? columnDimensions_[i] : 1;
        Real input = (i < inputDimensions_.size()) ? inputDimensions_[i] : 1;
        columnsPerInput += col / input;
    }
    return columnsPerInput / numDim;
}

Real SpatialPooler::avgConnectedSpanForColumn1D_(UInt column)
{

    NTA_ASSERT(inputDimensions_.size() == 1);
    std::vector<UInt> connectedSparse = connectedSynapses_.getSparseRow(column);
    if (connectedSparse.empty())
        return 0;
    auto minmax =
        minmax_element(connectedSparse.begin(), connectedSparse.end());
    return *minmax.second /*max*/ - *minmax.first /*min*/ + 1;
}

Real SpatialPooler::avgConnectedSpanForColumn2D_(UInt column)
{

    NTA_ASSERT(inputDimensions_.size() == 2);

    UInt nrows = inputDimensions_[0];
    UInt ncols = inputDimensions_[1];

    CoordinateConverter2D conv(nrows, ncols);

    std::vector<UInt> connectedSparse = connectedSynapses_.getSparseRow(column);
    std::vector<UInt> rows, cols;
    for (auto &elem : connectedSparse)
    {
        UInt index = elem;
        rows.push_back(conv.toRow(index));
        cols.push_back(conv.toCol(index));
    }

    if (rows.empty() && cols.empty())
    {
        return 0;
    }

    auto minmaxRows = minmax_element(rows.begin(), rows.end());
    UInt rowSpan = *minmaxRows.second /*max*/ - *minmaxRows.first /*min*/ + 1;

    auto minmaxCols = minmax_element(cols.begin(), cols.end());
    UInt colSpan = *minmaxCols.second - *minmaxCols.first + 1;

    return (rowSpan + colSpan) / 2.0;
}

Real SpatialPooler::avgConnectedSpanForColumnND_(UInt column)
{
    UInt numDimensions = inputDimensions_.size();
    std::vector<UInt> connectedSparse = connectedSynapses_.getSparseRow(column);
    std::vector<UInt> maxCoord(numDimensions, 0);
    std::vector<UInt> minCoord(
        numDimensions,
        *max_element(inputDimensions_.begin(), inputDimensions_.end()));

    CoordinateConverterND conv(inputDimensions_);

    if (connectedSparse.empty())
    {
        return 0;
    }

    std::vector<UInt> columnCoord;
    for (auto &elem : connectedSparse)
    {
        conv.toCoord(elem, columnCoord);
        for (size_t j = 0; j < columnCoord.size(); j++)
        {
            maxCoord[j] = std::max(maxCoord[j], columnCoord[j]);
            minCoord[j] = std::min(minCoord[j], columnCoord[j]);
        }
    }

    UInt totalSpan = 0;
    for (size_t j = 0; j < inputDimensions_.size(); j++)
    {
        totalSpan += maxCoord[j] - minCoord[j] + 1;
    }

    return (Real)totalSpan / inputDimensions_.size();
}

void SpatialPooler::adaptSynapses_(const UInt inputVector[],
                                   std::vector<UInt> &activeColumns)
{
    std::vector<Real> permChanges(numInputs_, -1 * synPermInactiveDec_);
    for (UInt i = 0; i < numInputs_; i++)
    {
        if (inputVector[i] > 0)
        {
            permChanges[i] = synPermActiveInc_;
        }
    }

    for (size_t i = 0; i < activeColumns.size(); i++)
    {
        UInt column = activeColumns[i];
        std::vector<UInt> potential;
        std::vector<Real> perm(numInputs_, 0);
        potential.resize(potentialPools_.nNonZerosOnRow(i));
        potential = potentialPools_.getSparseRow(column);
        permanences_.getRowToDense(column, perm);
        for (auto &elem : potential)
        {
            UInt index = elem;
            perm[index] += permChanges[index];
        }
        updatePermanencesForColumn_(perm, column, true);
    }
}

void SpatialPooler::bumpUpWeakColumns_()
{
    for (UInt i = 0; i < numColumns_; i++)
    {
        if (overlapDutyCycles_[i] >= minOverlapDutyCycles_[i])
        {
            continue;
        }
        std::vector<Real> perm(numInputs_, 0);
        std::vector<UInt> potential;
        potential.resize(potentialPools_.nNonZerosOnRow(i));
        potential = potentialPools_.getSparseRow(i);
        permanences_.getRowToDense(i, perm);
        for (auto &elem : potential)
        {
            UInt index = elem;
            perm[index] += synPermBelowStimulusInc_;
        }
        updatePermanencesForColumn_(perm, i, false);
    }
}

void SpatialPooler::updateDutyCyclesHelper_(std::vector<Real> &dutyCycles,
                                            std::vector<UInt> &newValues,
                                            UInt period)
{
    NTA_ASSERT(period >= 1);
    NTA_ASSERT(dutyCycles.size() == newValues.size());
    for (size_t i = 0; i < dutyCycles.size(); i++)
    {
        dutyCycles[i] =
            (dutyCycles[i] * static_cast<Real>(period - 1) + newValues[i]) /
            period;
    }
}

void SpatialPooler::updateBoostFactors_()
{
    if (globalInhibition_)
    {
        updateBoostFactorsGlobal_();
    }
    else
    {
        updateBoostFactorsLocal_();
    }
}

void SpatialPooler::updateBoostFactorsGlobal_()
{
    Real targetDensity;
    if (numActiveColumnsPerInhArea_ > 0)
    {
        UInt inhibitionArea = pow((Real)(2 * inhibitionRadius_ + 1),
                                  (Real)columnDimensions_.size());
        inhibitionArea = std::min(inhibitionArea, numColumns_);
        targetDensity = ((Real)numActiveColumnsPerInhArea_) / inhibitionArea;
        targetDensity = std::min(targetDensity, (Real)0.5);
    }
    else
    {
        targetDensity = localAreaDensity_;
    }

    for (UInt i = 0; i < numColumns_; ++i)
    {
        boostFactors_[i] =
            exp((targetDensity - activeDutyCycles_[i]) * boostStrength_);
    }
}

void SpatialPooler::updateBoostFactorsLocal_()
{
    for (UInt i = 0; i < numColumns_; ++i)
    {
        UInt numNeighbors = 0;
        Real localActivityDensity = 0;

        if (wrapAround_)
        {
            for (UInt neighbor :
                 WrappingNeighborhood(i, inhibitionRadius_, columnDimensions_))
            {
                localActivityDensity += activeDutyCycles_[neighbor];
                numNeighbors += 1;
            }
        }
        else
        {
            for (UInt neighbor :
                 Neighborhood(i, inhibitionRadius_, columnDimensions_))
            {
                localActivityDensity += activeDutyCycles_[neighbor];
                numNeighbors += 1;
            }
        }

        Real targetDensity = localActivityDensity / numNeighbors;
        boostFactors_[i] =
            exp((targetDensity - activeDutyCycles_[i]) * boostStrength_);
    }
}

void SpatialPooler::updateBookeepingVars_(bool learn)
{
    iterationNum_++;
    if (learn)
    {
        iterationLearnNum_++;
    }
}

void SpatialPooler::calculateOverlap_(UInt inputVector[],
                                      std::vector<UInt> &overlaps)
{
    overlaps.assign(numColumns_, 0);
    connectedSynapses_.rightVecSumAtNZ(inputVector, inputVector + numInputs_,
                                       overlaps.begin(), overlaps.end());
}

void SpatialPooler::calculateOverlapPct_(std::vector<UInt> &overlaps,
                                         std::vector<Real> &overlapPct)
{
    overlapPct.assign(numColumns_, 0);
    for (UInt i = 0; i < numColumns_; i++)
    {
        if (connectedCounts_[i] != 0)
        {
            overlapPct[i] = ((Real)overlaps[i]) / connectedCounts_[i];
        }
        else
        {
            // The intent here is to see if a cell matches its input well.
            // Therefore if nothing is connected the overlapPct is set to 0.
            overlapPct[i] = 0;
        }
    }
}

void SpatialPooler::inhibitColumns_(const std::vector<Real> &overlaps,
                                    std::vector<UInt> &activeColumns)
{
    Real density = localAreaDensity_;
    if (numActiveColumnsPerInhArea_ > 0)
    {
        UInt inhibitionArea = pow((Real)(2 * inhibitionRadius_ + 1),
                                  (Real)columnDimensions_.size());
        inhibitionArea = std::min(inhibitionArea, numColumns_);
        density = ((Real)numActiveColumnsPerInhArea_) / inhibitionArea;
        density = std::min(density, (Real)0.5);
    }

    if (globalInhibition_ ||
        inhibitionRadius_ >
            *max_element(columnDimensions_.begin(), columnDimensions_.end()))
    {
        inhibitColumnsGlobal_(overlaps, density, activeColumns);
    }
    else
    {
        inhibitColumnsLocal_(overlaps, density, activeColumns);
    }
}

bool SpatialPooler::isWinner_(Real score,
                              std::vector<std::pair<UInt, Real>> &winners,
                              UInt numWinners)
{
    if (score < stimulusThreshold_)
    {
        return false;
    }

    if (winners.size() < numWinners)
    {
        return true;
    }

    return score >= winners[numWinners - 1].second;
}

void SpatialPooler::addToWinners_(UInt index, Real score,
                                  std::vector<std::pair<UInt, Real>> &winners)
{
    std::pair<UInt, Real> val = std::make_pair(index, score);
    for (auto it = winners.begin(); it != winners.end(); it++)
    {
        if (score >= it->second)
        {
            winners.insert(it, val);
            return;
        }
    }
    winners.push_back(val);
}

void SpatialPooler::inhibitColumnsGlobal_(const std::vector<Real> &overlaps,
                                          Real density,
                                          std::vector<UInt> &activeColumns)
{
    activeColumns.clear();
    const UInt numDesired = (UInt)(density * numColumns_);
    NTA_CHECK(numDesired > 0) << "Not enough columns (" << numColumns_ << ") "
                              << "for desired density (" << density << ").";
    std::vector<std::pair<UInt, Real>> winners;
    for (UInt i = 0; i < numColumns_; i++)
    {
        if (isWinner_(overlaps[i], winners, numDesired))
        {
            addToWinners_(i, overlaps[i], winners);
        }
    }

    const UInt numActual = std::min(numDesired, (UInt)winners.size());
    for (UInt i = 0; i < numActual; i++)
    {
        activeColumns.push_back(winners[i].first);
    }
}

void SpatialPooler::inhibitColumnsLocal_(const std::vector<Real> &overlaps,
                                         Real density,
                                         std::vector<UInt> &activeColumns)
{
    activeColumns.clear();

    // Tie-breaking: when overlaps are equal, columns that have already been
    // selected are treated as "bigger".
    std::vector<bool> activeColumnsDense(numColumns_, false);

    for (UInt column = 0; column < numColumns_; column++)
    {
        if (overlaps[column] >= stimulusThreshold_)
        {
            UInt numNeighbors = 0;
            UInt numBigger = 0;

            if (wrapAround_)
            {
                for (UInt neighbor : WrappingNeighborhood(
                         column, inhibitionRadius_, columnDimensions_))
                {
                    if (neighbor != column)
                    {
                        numNeighbors++;

                        const Real difference =
                            overlaps[neighbor] - overlaps[column];
                        if (difference > 0 ||
                            (difference == 0 && activeColumnsDense[neighbor]))
                        {
                            numBigger++;
                        }
                    }
                }
            }
            else
            {
                for (UInt neighbor :
                     Neighborhood(column, inhibitionRadius_, columnDimensions_))
                {
                    if (neighbor != column)
                    {
                        numNeighbors++;

                        const Real difference =
                            overlaps[neighbor] - overlaps[column];
                        if (difference > 0 ||
                            (difference == 0 && activeColumnsDense[neighbor]))
                        {
                            numBigger++;
                        }
                    }
                }
            }

            UInt numActive = (UInt)(0.5 + (density * (numNeighbors + 1)));
            if (numBigger < numActive)
            {
                activeColumns.push_back(column);
                activeColumnsDense[column] = true;
            }
        }
    }
}

bool SpatialPooler::isUpdateRound_()
{
    return (iterationNum_ % updatePeriod_) == 0;
}

/* create a RNG with given seed */
void SpatialPooler::seed_(UInt64 seed) { rng_ = Random(seed); }

UInt SpatialPooler::persistentSize() const
{
    // TODO: this won't scale!
    std::stringstream s;
    s.flags(std::ios::scientific);
    s.precision(std::numeric_limits<double>::digits10 + 1);
    this->save(s);
    return s.str().size();
}

template <typename FloatType>
static void saveFloat_(std::ostream &outStream, FloatType v)
{
    outStream << std::setprecision(std::numeric_limits<FloatType>::max_digits10)
              << v << " ";
}

void SpatialPooler::save(std::ostream &outStream) const
{
    // Write a starting marker and version.
    outStream << "SpatialPooler" << std::endl;
    outStream << version_ << std::endl;

    // Store the simple variables first.
    outStream << numInputs_ << " " << numColumns_ << " " << potentialRadius_
              << " ";

    saveFloat_(outStream, potentialPct_);
    saveFloat_(outStream, initConnectedPct_);

    outStream << globalInhibition_ << " " << numActiveColumnsPerInhArea_ << " ";

    saveFloat_(outStream, localAreaDensity_);

    outStream << stimulusThreshold_ << " " << inhibitionRadius_ << " "
              << dutyCyclePeriod_ << " ";

    saveFloat_(outStream, boostStrength_);

    outStream << iterationNum_ << " " << iterationLearnNum_ << " "
              << spVerbosity_ << " " << updatePeriod_ << " ";

    saveFloat_(outStream, synPermMin_);
    saveFloat_(outStream, synPermMax_);
    saveFloat_(outStream, synPermTrimThreshold_);
    saveFloat_(outStream, synPermInactiveDec_);
    saveFloat_(outStream, synPermActiveInc_);
    saveFloat_(outStream, synPermBelowStimulusInc_);
    saveFloat_(outStream, synPermConnected_);
    saveFloat_(outStream, minPctOverlapDutyCycles_);

    outStream << wrapAround_ << " " << std::endl;

    // Store vectors.
    outStream << inputDimensions_.size() << " ";
    for (auto &elem : inputDimensions_)
    {
        outStream << elem << " ";
    }
    outStream << std::endl;

    outStream << columnDimensions_.size() << " ";
    for (auto &elem : columnDimensions_)
    {
        outStream << elem << " ";
    }
    outStream << std::endl;

    for (UInt i = 0; i < numColumns_; i++)
    {
        saveFloat_(outStream, boostFactors_[i]);
    }
    outStream << std::endl;

    for (UInt i = 0; i < numColumns_; i++)
    {
        saveFloat_(outStream, overlapDutyCycles_[i]);
    }
    outStream << std::endl;

    for (UInt i = 0; i < numColumns_; i++)
    {
        saveFloat_(outStream, activeDutyCycles_[i]);
    }
    outStream << std::endl;

    for (UInt i = 0; i < numColumns_; i++)
    {
        saveFloat_(outStream, minOverlapDutyCycles_[i]);
    }
    outStream << std::endl;

    for (UInt i = 0; i < numColumns_; i++)
    {
        outStream << tieBreaker_[i] << " ";
    }
    outStream << std::endl;

    // Store matrices.
    for (UInt i = 0; i < numColumns_; i++)
    {
        std::vector<UInt> pot;
        pot.resize(potentialPools_.nNonZerosOnRow(i));
        pot = potentialPools_.getSparseRow(i);
        outStream << pot.size() << std::endl;
        for (auto &elem : pot)
        {
            outStream << elem << " ";
        }
        outStream << std::endl;
    }
    outStream << std::endl;

    for (UInt i = 0; i < numColumns_; i++)
    {
        std::vector<std::pair<UInt, Real>> perm;
        perm.resize(permanences_.nNonZerosOnRow(i));
        outStream << perm.size() << std::endl;
        permanences_.getRowToSparse(i, perm.begin());
        for (auto &elem : perm)
        {
            outStream << elem.first << " ";
            saveFloat_(outStream, elem.second);
        }
        outStream << std::endl;
    }
    outStream << std::endl;

    outStream << rng_ << std::endl;

    outStream << "~SpatialPooler" << std::endl;
}

// Implementation note: this method sets up the instance using data from
// inStream. This method does not call initialize. As such we have to be careful
// that everything in initialize is handled properly here.
void SpatialPooler::load(std::istream &inStream)
{
    // Current version
    version_ = 2;

    // Check the marker
    std::string marker;
    inStream >> marker;
    NTA_CHECK(marker == "SpatialPooler");

    // Check the saved version.
    UInt version;
    inStream >> version;
    NTA_CHECK(version <= version_);

    // Retrieve simple variables
    inStream >> numInputs_ >> numColumns_ >> potentialRadius_ >>
        potentialPct_ >> initConnectedPct_ >> globalInhibition_ >>
        numActiveColumnsPerInhArea_ >> localAreaDensity_ >>
        stimulusThreshold_ >> inhibitionRadius_ >> dutyCyclePeriod_ >>
        boostStrength_ >> iterationNum_ >> iterationLearnNum_ >> spVerbosity_ >>
        updatePeriod_

        >> synPermMin_ >> synPermMax_ >> synPermTrimThreshold_ >>
        synPermInactiveDec_ >> synPermActiveInc_ >> synPermBelowStimulusInc_ >>
        synPermConnected_ >> minPctOverlapDutyCycles_;
    if (version == 1)
    {
        wrapAround_ = true;
    }
    else
    {
        inStream >> wrapAround_;
    }

    // Retrieve vectors.
    UInt numInputDimensions;
    inStream >> numInputDimensions;
    inputDimensions_.resize(numInputDimensions);
    for (UInt i = 0; i < numInputDimensions; i++)
    {
        inStream >> inputDimensions_[i];
    }

    UInt numColumnDimensions;
    inStream >> numColumnDimensions;
    columnDimensions_.resize(numColumnDimensions);
    for (UInt i = 0; i < numColumnDimensions; i++)
    {
        inStream >> columnDimensions_[i];
    }

    boostFactors_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        inStream >> boostFactors_[i];
    }

    overlapDutyCycles_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        inStream >> overlapDutyCycles_[i];
    }

    activeDutyCycles_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        inStream >> activeDutyCycles_[i];
    }

    minOverlapDutyCycles_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        inStream >> minOverlapDutyCycles_[i];
    }

    tieBreaker_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        inStream >> tieBreaker_[i];
    }

    // Retrieve matrices.
    potentialPools_.resize(numColumns_, numInputs_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        UInt nNonZerosOnRow;
        inStream >> nNonZerosOnRow;
        std::vector<UInt> pot(nNonZerosOnRow, 0);
        for (UInt j = 0; j < nNonZerosOnRow; j++)
        {
            inStream >> pot[j];
        }
        potentialPools_.replaceSparseRow(i, pot.begin(), pot.end());
    }

    permanences_.resize(numColumns_, numInputs_);
    connectedSynapses_.resize(numColumns_, numInputs_);
    connectedCounts_.resize(numColumns_);
    for (UInt i = 0; i < numColumns_; i++)
    {
        UInt nNonZerosOnRow;
        inStream >> nNonZerosOnRow;
        std::vector<Real> perm(numInputs_, 0);

        for (UInt j = 0; j < nNonZerosOnRow; j++)
        {
            UInt index;
            Real value;
            inStream >> index;
            inStream >> value;
            perm[index] = value;
        }
        updatePermanencesForColumn_(perm, i, false);
    }

    inStream >> rng_;

    inStream >> marker;
    NTA_CHECK(marker == "~SpatialPooler");

    // initialize ephemeral members
    overlaps_.resize(numColumns_);
    overlapsPct_.resize(numColumns_);
    boostedOverlaps_.resize(numColumns_);
}

//----------------------------------------------------------------------
// Debugging helpers
//----------------------------------------------------------------------

// Print the main SP creation parameters
void SpatialPooler::printParameters() const
{
    std::cout
        << "------------CPP SpatialPooler Parameters ------------------\n";
    std::cout
        << "iterationNum                = " << getIterationNum() << std::endl
        << "iterationLearnNum           = " << getIterationLearnNum()
        << std::endl
        << "numInputs                   = " << getNumInputs() << std::endl
        << "numColumns                  = " << getNumColumns() << std::endl
        << "numActiveColumnsPerInhArea  = " << getNumActiveColumnsPerInhArea()
        << std::endl
        << "potentialPct                = " << getPotentialPct() << std::endl
        << "globalInhibition            = " << getGlobalInhibition()
        << std::endl
        << "localAreaDensity            = " << getLocalAreaDensity()
        << std::endl
        << "stimulusThreshold           = " << getStimulusThreshold()
        << std::endl
        << "synPermActiveInc            = " << getSynPermActiveInc()
        << std::endl
        << "synPermInactiveDec          = " << getSynPermInactiveDec()
        << std::endl
        << "synPermConnected            = " << getSynPermConnected()
        << std::endl
        << "minPctOverlapDutyCycles     = " << getMinPctOverlapDutyCycles()
        << std::endl
        << "dutyCyclePeriod             = " << getDutyCyclePeriod() << std::endl
        << "boostStrength               = " << getBoostStrength() << std::endl
        << "spVerbosity                 = " << getSpVerbosity() << std::endl
        << "wrapAround                  = " << getWrapAround() << std::endl
        << "version                     = " << version() << std::endl;
}

void SpatialPooler::printState(std::vector<UInt> &state)
{
    std::cout << "[  ";
    for (UInt i = 0; i != state.size(); ++i)
    {
        if (i > 0 && i % 10 == 0)
        {
            std::cout << "\n   ";
        }
        std::cout << state[i] << " ";
    }
    std::cout << "]\n";
}

void SpatialPooler::printState(std::vector<Real> &state)
{
    std::cout << "[  ";
    for (UInt i = 0; i != state.size(); ++i)
    {
        if (i > 0 && i % 10 == 0)
        {
            std::cout << "\n   ";
        }
        std::printf("%6.3f ", state[i]);
    }
    std::cout << "]\n";
}

} // namespace crucian
#pragma clang diagnostic pop
