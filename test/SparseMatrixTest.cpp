/* ---------------------------------------------------------------------
 * Numenta Platform for Intelligent Computing (NuPIC)
 * Copyright (C) 2013-2016, Numenta, Inc.  Unless you have an agreement
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
 * Unit tests for SparseMatrix class
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <crucian/SparseMatrix.hpp>

namespace crucian
{

struct IncrementNonZerosOnOuterTest
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> outerRows;
    std::vector<UInt32> outerCols;
    Real32 delta;
    std::vector<Real32> expected;
};

TEST(SparseMatrixTest, incrementNonZerosOnOuter)
{
    std::vector<IncrementNonZerosOnOuterTest> tests = {
        {"Test 1",
         // Dimensions
         4,
         4,
         // Before
         {0, 1, 0, 1, 2, 0, 2, 0, 0, 1, 0, 1, 2, 0, 2, 0},
         // Selection
         {0, 2, 3},
         {0, 1},
         // Delta
         40,
         // Expected
         {0, 41, 0, 1, 2, 0, 2, 0, 0, 41, 0, 1, 42, 0, 2, 0}},
        {"Test 2",
         // Dimensions
         4,
         4,
         // Before
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
         // Selection
         {0, 3},
         {0, 3},
         // Delta
         41,
         // Expected
         {42, 1, 1, 42, 1, 1, 1, 1, 1, 1, 1, 1, 42, 1, 1, 42}},
        {"Test 3",
         // Dimensions
         4,
         4,
         // Before
         {0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0},
         // Selection
         {0, 3},
         {0, 3},
         // Delta
         41,
         // Expected
         {0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0}}};

    for (const IncrementNonZerosOnOuterTest &test : tests)
    {
        NTA_INFO << "Test: " << test.name;
        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.incrementNonZerosOnOuter(test.outerRows.begin(), test.outerRows.end(),
                                   test.outerCols.begin(), test.outerCols.end(),
                                   test.delta);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        EXPECT_EQ(test.expected, actual);
    }
}

struct IncrementNonZerosOnRowsExcludingColsTest
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> outerRows;
    std::vector<UInt32> outerCols;
    Real32 delta;
    std::vector<Real32> expected;
};

TEST(SparseMatrixTest, incrementNonZerosOnRowsExcludingCols)
{
    std::vector<IncrementNonZerosOnRowsExcludingColsTest> tests = {
        {"Test 1",
         // Dimensions
         4,
         4,
         // Before
         {0, 1, 0, 1, 2, 0, 2, 0, 0, 1, 0, 1, 2, 0, 2, 0},
         // Selection
         {0, 2, 3},
         {0, 1},
         // Delta
         40,
         // Expected
         {0, 1, 0, 41, 2, 0, 2, 0, 0, 1, 0, 41, 2, 0, 42, 0}},
        {"Test 2",
         // Dimensions
         4,
         4,
         // Before
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
         // Selection
         {0, 3},
         {0, 3},
         // Delta
         41,
         // Expected
         {1, 42, 42, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 42, 42, 1}},
        {"Test 3",
         // Dimensions
         4,
         4,
         // Before
         {1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
         // Selection
         {0, 3},
         {0, 3},
         // Delta
         41,
         // Expected
         {1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1}}};

    for (const IncrementNonZerosOnRowsExcludingColsTest &test : tests)
    {
        NTA_INFO << "Test: " << test.name;
        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.incrementNonZerosOnRowsExcludingCols(
            test.outerRows.begin(), test.outerRows.end(),
            test.outerCols.begin(), test.outerCols.end(), test.delta);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        EXPECT_EQ(test.expected, actual);
    }
}

struct SetZerosOnOuterTest
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> outerRows;
    std::vector<UInt32> outerCols;
    Real32 value;
    std::vector<Real32> expected;
};

TEST(SparseMatrixTest, setZerosOnOuter)
{
    std::vector<SetZerosOnOuterTest> tests = {
        {"Test 1",
         // Dimensions
         4,
         4,
         // Before
         {0, 1, 0, 1, 2, 0, 2, 0, 0, 1, 0, 1, 2, 0, 2, 0},
         // Selection
         {0, 2, 3},
         {0, 1},
         // Value
         42,
         // Expected
         {42, 1, 0, 1, 2, 0, 2, 0, 42, 1, 0, 1, 2, 42, 2, 0}},
        {"Test 2",
         // Dimensions
         4,
         4,
         // Before
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
         // Selection
         {0, 3},
         {0, 3},
         // Value
         42,
         // Expected
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
        {"Test 3",
         // Dimensions
         4,
         4,
         // Before
         {1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
         // Selection
         {0, 3},
         {1, 2},
         // Value
         42,
         // Expected
         {1, 42, 42, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 42, 42, 1}}};

    for (const SetZerosOnOuterTest &test : tests)
    {
        NTA_INFO << "Test: " << test.name;
        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.setZerosOnOuter(test.outerRows.begin(), test.outerRows.end(),
                          test.outerCols.begin(), test.outerCols.end(),
                          test.value);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        EXPECT_EQ(test.expected, actual);
    }
}

struct SetRandomZerosOnOuterTest_single
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> outerRows;
    std::vector<UInt32> outerCols;
    Int32 numNewNonZerosPerRow;
    Real32 value;
};

TEST(SparseMatrixTest, setRandomZerosOnOuter_single)
{
    Random rng;

    std::vector<SetRandomZerosOnOuterTest_single> tests = {
        {"Test 1",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {0, 3, 4, 5, 6, 7},
         {0, 3, 4},
         // numNewNonZerosPerRow
         2,
         // value
         42},
        {"No selected rows",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {},
         {0, 3, 4},
         // numNewNonZerosPerRow
         2,
         // value
         42},
        {"No selected cols",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {0, 3, 4, 5, 6, 7},
         {},
         // numNewNonZerosPerRow
         2,
         // value
         42}};

    for (const SetRandomZerosOnOuterTest_single &test : tests)
    {
        NTA_INFO << "Test: " << test.name;

        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.setRandomZerosOnOuter(test.outerRows.begin(), test.outerRows.end(),
                                test.outerCols.begin(), test.outerCols.end(),
                                test.numNewNonZerosPerRow, test.value, rng);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        for (UInt32 row = 0; row < test.nrows; row++)
        {
            Int32 numSelectedZeros = 0;
            Int32 numConvertedZeros = 0;

            for (UInt32 col = 0; col < test.ncols; col++)
            {
                UInt32 i = row * test.ncols + col;
                if (std::binary_search(test.outerRows.begin(),
                                       test.outerRows.end(), row) &&
                    std::binary_search(test.outerCols.begin(),
                                       test.outerCols.end(), col))
                {
                    if (test.before[i] == 0)
                    {
                        numSelectedZeros++;

                        if (actual[i] != 0)
                        {
                            // Should replace zeros with the specified value.
                            EXPECT_EQ(test.value, actual[i]);
                            numConvertedZeros++;
                        }
                    }
                }
                else
                {
                    // Every value not in the selection should be unchanged.
                    EXPECT_EQ(test.before[i], actual[i]);
                }

                if (test.before[i] != 0)
                {
                    // Every value that was nonzero should not have changed.
                    EXPECT_EQ(test.before[i], actual[i]);
                }
            }

            // Should replace numNewNonZerosPerRow, or all of them if there
            // aren't that many.
            EXPECT_EQ(std::min(test.numNewNonZerosPerRow, numSelectedZeros),
                      numConvertedZeros);
        }
    }
}

struct SetRandomZerosOnOuterTest_multi
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> outerRows;
    std::vector<UInt32> outerCols;
    std::vector<Int32> numNewNonZerosPerRow;
    Real32 value;
};

TEST(SparseMatrixTest, setRandomZerosOnOuter_multi)
{
    Random rng;

    std::vector<SetRandomZerosOnOuterTest_multi> tests = {
        {"Test 1",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {0, 3, 4, 5, 6, 7},
         {0, 3, 4},
         // numNewNonZerosPerRow
         {2, 2, 2, 2, 2, 2},
         // value
         42},
        {"No selected rows",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {},
         {0, 3, 4},
         // numNewNonZerosPerRow
         {},
         // value
         42},
        {"No selected cols",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {0, 3, 4, 5, 6, 7},
         {},
         // numNewNonZerosPerRow
         {2, 2, 2, 2, 2, 2},
         // value
         42}};

    for (const SetRandomZerosOnOuterTest_multi &test : tests)
    {
        NTA_INFO << "Test: " << test.name;

        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.setRandomZerosOnOuter(test.outerRows.begin(), test.outerRows.end(),
                                test.outerCols.begin(), test.outerCols.end(),
                                test.numNewNonZerosPerRow.begin(),
                                test.numNewNonZerosPerRow.end(), test.value,
                                rng);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        for (UInt32 row = 0; row < test.nrows; row++)
        {
            Int32 numSelectedZeros = 0;
            Int32 numConvertedZeros = 0;

            auto rowInSelection =
                std::find(test.outerRows.begin(), test.outerRows.end(), row);
            const Int32 requestedNumNewZeros =
                rowInSelection != test.outerRows.end()
                    ? test.numNewNonZerosPerRow[rowInSelection -
                                                test.outerRows.begin()]
                    : 0;

            for (UInt32 col = 0; col < test.ncols; col++)
            {
                UInt32 i = row * test.ncols + col;
                if (rowInSelection != test.outerRows.end() &&
                    std::binary_search(test.outerCols.begin(),
                                       test.outerCols.end(), col))
                {
                    if (test.before[i] == 0)
                    {
                        numSelectedZeros++;

                        if (actual[i] != 0)
                        {
                            // Should replace zeros with the specified value.
                            EXPECT_EQ(test.value, actual[i]);
                            numConvertedZeros++;
                        }
                    }
                }
                else
                {
                    // Every value not in the selection should be unchanged.
                    EXPECT_EQ(test.before[i], actual[i]);
                }

                if (test.before[i] != 0)
                {
                    // Every value that was nonzero should not have changed.
                    EXPECT_EQ(test.before[i], actual[i]);
                }
            }

            // Should replace numNewNonZerosPerRow, or all of them if there
            // aren't that many.
            EXPECT_EQ(std::min(requestedNumNewZeros, numSelectedZeros),
                      numConvertedZeros);
        }
    }
}

struct IncreaseRowNonZeroCountsOnOuterToTest
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> outerRows;
    std::vector<UInt32> outerCols;
    int numDesiredNonzeros;
    Real32 value;
};

TEST(SparseMatrixTest, increaseRowNonZeroCountsOnOuterTo)
{
    Random rng;

    std::vector<IncreaseRowNonZeroCountsOnOuterToTest> tests = {
        {"Test 1",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {0, 3, 4, 5, 6, 7},
         {0, 3, 4},
         // numDesiredNonzeros
         2,
         // value
         42},
        {"No selected rows",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {},
         {0, 3, 4},
         // numDesiredNonzeros
         2,
         // value
         42},
        {"No selected cols",
         // Dimensions
         8,
         6,
         // Before
         {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
          0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
          0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1},
         // Selection
         {0, 3, 4, 5, 6, 7},
         {},
         // numDesiredNonzeros
         2,
         // value
         42},
        {"Try to catch unsigned integer bugs",
         // Dimensions
         2,
         4,
         // Before
         {1, 1, 0, 0, 1, 1, 1, 0},
         // Selection
         {0, 1},
         {0, 1, 2, 3},
         // numDesiredNonzeros
         2,
         // value
         42}};

    for (const IncreaseRowNonZeroCountsOnOuterToTest &test : tests)
    {
        NTA_INFO << "Test: " << test.name;

        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.increaseRowNonZeroCountsOnOuterTo(
            test.outerRows.begin(), test.outerRows.end(),
            test.outerCols.begin(), test.outerCols.end(),
            test.numDesiredNonzeros, test.value, rng);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        for (UInt32 row = 0; row < test.nrows; row++)
        {
            int numSelectedZeros = 0;
            int numConvertedZeros = 0;

            for (UInt32 col = 0; col < test.ncols; col++)
            {
                UInt32 i = row * test.ncols + col;
                if (std::binary_search(test.outerRows.begin(),
                                       test.outerRows.end(), row) &&
                    std::binary_search(test.outerCols.begin(),
                                       test.outerCols.end(), col))
                {
                    if (test.before[i] == 0)
                    {
                        numSelectedZeros++;

                        if (actual[i] != 0)
                        {
                            // Should replace zeros with the specified value.
                            EXPECT_EQ(test.value, actual[i]);
                            numConvertedZeros++;
                        }
                    }
                }
                else
                {
                    // Every value not in the selection should be unchanged.
                    EXPECT_EQ(test.before[i], actual[i]);
                }

                if (test.before[i] != 0)
                {
                    // Every value that was nonzero should not have changed.
                    EXPECT_EQ(test.before[i], actual[i]);
                }
            }

            int numSelectedNonZeros = test.outerCols.size() - numSelectedZeros;
            int numDesiredToAdd =
                std::max(0, test.numDesiredNonzeros - numSelectedNonZeros);
            int numToAdd = std::min(numSelectedZeros, numDesiredToAdd);
            EXPECT_EQ(numToAdd, numConvertedZeros);
        }
    }
}

struct ClipRowsBelowAndAboveTest
{
    std::string name;
    UInt32 nrows;
    UInt32 ncols;
    std::vector<Real32> before;
    std::vector<UInt32> selectedRows;
    Real32 lower;
    Real32 upper;
    std::vector<Real32> expected;
};

TEST(SparseMatrixTest, clipRowsBelowAndAbove)
{
    std::vector<ClipRowsBelowAndAboveTest> tests = {
        {"Test 1",
         // Dimensions
         3,
         5,
         // Before
         {-5, -4, 0.5, 4, 5, -5, -4, 0.5, 4, 5, -5, -4, 0.5, 4, 5},
         // Selection
         {0, 2},
         // Boundaries
         -4,
         4,
         // Expected
         {-4, -4, 0.5, 4, 4, -5, -4, 0.5, 4, 5, -4, -4, 0.5, 4, 4}},
        {"Test 2",
         // Dimensions
         3,
         5,
         // Before
         {-5, -4, 0.5, 4, 5, -5, -4, 0.5, 4, 5, -5, -4, 0.5, 4, 5},
         // Selection
         {1, 2},
         // Boundaries
         0,
         3,
         // Expected
         {-5, -4, 0.5, 4, 5, 0, 0, 0.5, 3, 3, 0, 0, 0.5, 3, 3}}};

    for (const ClipRowsBelowAndAboveTest &test : tests)
    {
        NTA_INFO << "Test: " << test.name;
        SparseMatrix<> m(test.nrows, test.ncols, test.before.begin());

        m.clipRowsBelowAndAbove(test.selectedRows.begin(),
                                test.selectedRows.end(), test.lower,
                                test.upper);

        std::vector<Real32> actual(test.nrows * test.ncols);
        m.toDense(actual.begin());

        EXPECT_EQ(test.expected, actual);
    }
}

} // namespace crucian
