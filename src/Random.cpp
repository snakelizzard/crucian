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
    Random Number Generator implementation
*/

#include <cmath> // For ldexp.
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream> // for istream, ostream
#include <limits>

#include <crucian/Log.hpp>
#include <crucian/Random.hpp>

namespace crucian
{

Random *Random::theInstanceP_ = nullptr;
RandomSeedFuncPtr Random::seeder_ = nullptr;

const UInt32 Random::MAX32 = std::numeric_limits<UInt32>::max();
const UInt64 Random::MAX64 = std::numeric_limits<UInt64>::max();

static NTA_UInt64 badSeeder()
{
    NTA_THROW << "Logic error in initialization of Random subsystem.";
    return 0;
}

/**
 * Using an Impl provides two things:
 * 1) ability to specify different algorithms (not yet implemented)
 * 2) constructors Random(long) and Random(string) without code duplication.
 */

// Algorithm-level implementation of the random number generator.
// When we have different algorithms RandomImpl will become an interface
// class and subclasses will implement specific algorithms

class RandomImpl
{
public:
    RandomImpl(UInt64 seed);

    UInt32 getUInt32();

    bool operator==(const RandomImpl &o) const;

    inline bool operator!=(const RandomImpl &other) const
    {
        return !operator==(other);
    }
    // Note: copy constructor and operator= are needed
    // The default is ok.
private:
    friend std::ostream &operator<<(std::ostream &outStream,
                                    const RandomImpl &r);

    friend std::istream &operator>>(std::istream &inStream, RandomImpl &r);

    const static UInt32 VERSION = 2;
    // internal state
    static const int stateSize_ = 31;
    static const int sep_ = 3;
    UInt32 state_[stateSize_];
    int rptr_;
    int fptr_;
};

Random::Random(const Random &r)
{
    NTA_CHECK(r.impl_ != nullptr);
    seed_ = r.seed_;
    impl_ = new RandomImpl(*r.impl_);
}

void Random::reseed(UInt64 seed)
{
    seed_ = seed;
    if (impl_)
        delete impl_;
    impl_ = new RandomImpl(seed);
}

Random &Random::operator=(const Random &other)
{
    if (this != &other)
    {
        seed_ = other.seed_;
        if (impl_)
            delete impl_;
        NTA_CHECK(other.impl_ != nullptr);
        impl_ = new RandomImpl(*other.impl_);
    }
    return *this;
}

bool Random::operator==(const Random &o) const
{
    return seed_ == o.seed_ && (*impl_) == (*o.impl_);
}

Random::~Random() { delete impl_; }

Random::Random(UInt64 seed)
{
    // Get the seeder even if we don't need it, because
    // this will have the side effect of allocating the
    // singleton if necessary. The singleton will actuallly
    // be allocated in a recursive call to the Random
    // constructor, with seed = 0 and
    RandomSeedFuncPtr seeder = getSeeder();
    NTA_CHECK(seeder != nullptr);
    if (seed == 0)
    {
        if (seeder == badSeeder)
        {
            // we are constructing the singleton
            seed_ = static_cast<UInt64>(time(nullptr));
        }
        else
        {
            seed_ = seeder();
        }
    }
    else
    {
        seed_ = seed;
    }
    // if seed is zero at this point, there is a logic error.
    NTA_CHECK(seed_ != 0);
    impl_ = new RandomImpl(seed_);
}

RandomSeedFuncPtr Random::getSeeder()
{
    if (seeder_ == nullptr)
    {
        NTA_CHECK(theInstanceP_ == nullptr);
        // set the seeder to something not NULL
        // so the constructor below will not
        // see a NULL pointer and call us recursively.
        seeder_ = badSeeder;
        theInstanceP_ = new Random(0);
        seeder_ = GetRandomSeed;
    }
    return seeder_;
}

void Random::initSeeder(const RandomSeedFuncPtr r)
{
    NTA_CHECK(r != nullptr);
    seeder_ = r;
}

void Random::shutdown()
{
    if (theInstanceP_ != nullptr)
    {
        delete theInstanceP_;
        theInstanceP_ = nullptr;
    }
}

UInt32 Random::getUInt32(const UInt32 max)
{
    NTA_ASSERT(max > 0);
    UInt32 smax = Random::MAX32 - (Random::MAX32 % max);
    UInt32 sample;
    do
    {
        sample = impl_->getUInt32();
    } while (sample > smax);

    // NTA_WARN << "Random32(" << max << ") -> " << sample % max << " smax = "
    // << smax;
    return sample % max;
}

UInt64 Random::getUInt64(const UInt64 max)
{
    NTA_ASSERT(max > 0);
    UInt64 smax = Random::MAX64 - (Random::MAX64 % max);
    UInt64 sample, lo, hi;
    do
    {
        lo = impl_->getUInt32();
        hi = impl_->getUInt32();
        sample = lo | (hi << 32);
    } while (sample > smax);
    // NTA_WARN << "Random64(" << max << ") -> " << sample % max << " smax = "
    // << smax;

    return sample % max;
}

double Random::getReal64()
{
    constexpr int mantissaBits = 48;
    const UInt64 max = static_cast<UInt64>(0x1U) << mantissaBits;
    UInt64 value = getUInt64(max);
    Real64 dvalue = static_cast<Real64>(
        value); // No loss because we only need the 48 mantissa bits.
    Real64 returnval = ::ldexp(dvalue, -mantissaBits);
    // NTA_WARN << "RandomReal -> " << returnval;
    return returnval;
}

// ---- RandomImpl follows ----

UInt32 RandomImpl::getUInt32(void)
{
    UInt32 i;
#ifdef RANDOM_SUPERDEBUG
    printf("Random::get *fptr = %ld; *rptr = %ld fptr = %ld rptr = %ld\n",
           state_[fptr_], state_[rptr_], fptr_, rptr_);
#endif
    state_[fptr_] = static_cast<UInt32>((static_cast<UInt64>(state_[fptr_]) +
                                         static_cast<UInt64>(state_[rptr_])) %
                                        Random::MAX32);
    i = state_[fptr_];
    i = (i >> 1) & 0x7fffffff; /* chucking least random bit */
    if (++fptr_ >= stateSize_)
    {
        fptr_ = 0;
        ++rptr_;
    }
    else if (++rptr_ >= stateSize_)
        rptr_ = 0;
#ifdef RANDOM_SUPERDEBUG
    printf("Random::get returning %ld\n", i);
    for (int j = 0; j < stateSize_; j++)
    {
        printf("Random:get: %d  %ld\n", j, state_[j]);
    }
#endif

    return i;
}

RandomImpl::RandomImpl(UInt64 seed)
{

    /**
     * Initialize our state. Taken from BSD source for random()
     */
    state_[0] = static_cast<UInt32>(seed % Random::MAX32);
    for (long i = 1; i < stateSize_; i++)
    {
        /*
         * Implement the following, without overflowing 31 bits:
         *
         *	state[i] = (16807 * state[i - 1]) % 2147483647;
         *
         *	2^31-1 (prime) = 2147483647 = 127773*16807+2836
         */
        Int32 quot = state_[i - 1] / 127773;
        Int32 rem = state_[i - 1] % 127773;
        Int32 test = 16807 * rem - 2836 * quot;
        state_[i] = static_cast<UInt32>(
            (static_cast<UInt64>(test) + (test < 0 ? 2147483647 : 0)) %
            Random::MAX32);
    }
    fptr_ = sep_;
    rptr_ = 0;
#ifdef RANDOM_SUPERDEBUG
    printf("Random: init for seed = %lu\n", seed);
    for (int i = 0; i < stateSize_; i++)
    {
        printf("Random: %d  %ld\n", i, state_[i]);
    }
#endif

    for (long i = 0; i < 10 * stateSize_; i++)
        (void)getUInt32();
#ifdef RANDOM_SUPERDEBUG
    printf("Random: after init for seed = %lu\n", seed);
    printf("Random: *fptr = %ld; *rptr = %ld fptr = %ld rptr = %ld\n",
           state_[fptr_], state_[rptr_], fptr_, rptr_);
    for (long i = 0; i < stateSize_; i++)
    {
        printf("Random: %d  %ld\n", i, state_[i]);
    }
#endif
}

std::ostream &operator<<(std::ostream &outStream, const Random &r)
{
    outStream << "random-v1 ";
    outStream << r.seed_ << " ";
    NTA_CHECK(r.impl_ != nullptr);
    outStream << *r.impl_;
    outStream << " endrandom-v1";
    return outStream;
}

std::istream &operator>>(std::istream &inStream, Random &r)
{
    std::string version;

    inStream >> version;
    if (version != "random-v1")
    {
        NTA_THROW
            << "Random() deserializer -- found unexpected version string '"
            << version << "'";
    }
    inStream >> r.seed_;
    if (!r.impl_)
        r.impl_ = new RandomImpl(0);

    inStream >> *r.impl_;

    std::string endtag;
    inStream >> endtag;
    if (endtag != "endrandom-v1")
    {
        NTA_THROW << "Random() deserializer -- found unexpected end tag '"
                  << endtag << "'";
    }

    return inStream;
}

std::ostream &operator<<(std::ostream &outStream, const RandomImpl &r)
{
    outStream << "RandomImpl " << RandomImpl::VERSION << " ";
    outStream << RandomImpl::stateSize_ << " ";
    for (auto &elem : r.state_)
    {
        outStream << elem << " ";
    }
    outStream << r.rptr_ << " ";
    outStream << r.fptr_;
    return outStream;
}

std::istream &operator>>(std::istream &inStream, RandomImpl &r)
{
    std::string marker;
    inStream >> marker;
    UInt32 version;
    if (marker == "RandomImpl")
    {
        inStream >> version;
        if (version != 2)
        {
            NTA_THROW << "RandomImpl deserialization found unexpected version: "
                      << version;
        }
    }
    else if (marker == "randomimpl-v1")
    {
        version = 1;
    }
    else
    {
        NTA_THROW << "RandomImpl() deserializer -- found unexpected version "
                  << "string '" << marker << "'";
    }
    UInt32 ss = 0;
    inStream >> ss;
    NTA_CHECK(ss == static_cast<UInt32>(RandomImpl::stateSize_))
        << " ss = " << ss;

    int tmp;
    for (auto &elem : r.state_)
    {
        if (version < 2)
        {
            inStream >> tmp;
            elem = static_cast<UInt32>(tmp);
        }
        else
        {
            inStream >> elem;
        }
    }
    inStream >> r.rptr_;
    inStream >> r.fptr_;
    return inStream;
}

bool RandomImpl::operator==(const RandomImpl &o) const
{
    if (rptr_ != o.rptr_ || fptr_ != o.fptr_)
    {
        return false;
    }
    return std::memcmp(state_, o.state_, sizeof(state_)) == 0;
}

// helper function for seeding RNGs across the plugin barrier
// Unless there is a logic error, should not be called if
// the Random singleton has not been initialized.
NTA_UInt64 GetRandomSeed()
{
    Random *r = crucian::Random::theInstanceP_;
    NTA_CHECK(r != nullptr);
    NTA_UInt64 result = r->getUInt64();
    return result;
}

} // namespace crucian
