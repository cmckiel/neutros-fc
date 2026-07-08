// time_util_test.cpp
//
// GoogleTest suite for:
//     uint32_t time_util_calc_ticks_elapsed(uint32_t time_start, uint32_t time_now);
//
// The contract under test: elapsed ticks are computed in modulo-2^32
// arithmetic, so a single counter rollover (time_now < time_start because
// the hardware counter wrapped past UINT32_MAX) yields the correct positive
// delta. This is valid as long as fewer than 2^32 ticks actually elapsed.

#include <gtest/gtest.h>
#include <cstdint>

extern "C" {
  #include "time_util.h"
}

namespace {

// ===========================================================================
// Named cases: the "no rollover" happy path
// ===========================================================================

TEST(CalcTicksElapsed, ZeroWhenStartEqualsNow) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(0u, 0u), 0u);
    EXPECT_EQ(time_util_calc_ticks_elapsed(100u, 100u), 0u);
    EXPECT_EQ(time_util_calc_ticks_elapsed(UINT32_MAX, UINT32_MAX), 0u);
}

TEST(CalcTicksElapsed, SimpleForwardDelta) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(0u, 100u), 100u);
    EXPECT_EQ(time_util_calc_ticks_elapsed(1000u, 1250u), 250u);
    EXPECT_EQ(time_util_calc_ticks_elapsed(0x00001000u, 0x00001001u), 1u);
}

// ===========================================================================
// Named cases: rollover / boundary behavior (the whole point)
// ===========================================================================

// Counter wrapped: start is just below the top, now is just above zero.
TEST(CalcTicksElapsed, ClassicRolloverAcrossBoundary) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(0xFFFFFFF0u, 0x00000010u), 0x20u);  // 32 ticks
}

// The tightest rollover: one tick takes MAX -> 0.
TEST(CalcTicksElapsed, SingleTickAcrossWrap) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(UINT32_MAX, 0u), 1u);
}

// now < start with no boundary crossing at the value level still yields the
// modular result — the function cannot distinguish "went backwards" from
// "wrapped almost all the way around". Both are the same bit pattern.
TEST(CalcTicksElapsed, NowLessThanStartWrapsToLargeDelta) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(0x0000000Au, 0x00000005u), 0xFFFFFFFBu);  // -5 mod 2^32
}

// Largest representable elapsed count: a full range minus one tick.
TEST(CalcTicksElapsed, MaxRepresentableElapsed) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(1u, 0u), UINT32_MAX);
}

// Wraps centered on the signed-boundary, where a naive signed impl would break.
TEST(CalcTicksElapsed, SignedMidpointBoundaries) {
    EXPECT_EQ(time_util_calc_ticks_elapsed(0x80000000u, 0x7FFFFFFFu), UINT32_MAX);
    EXPECT_EQ(time_util_calc_ticks_elapsed(0x7FFFFFFFu, 0x80000000u), 1u);
}

// ===========================================================================
// Value-parameterized property test:
//   For ANY start and ANY delta, elapsed(start, start + delta) == delta,
//   with start + delta computed in wrapping uint32 arithmetic.
// This exercises every combination of wrapping/non-wrapping start points and
// small/large/boundary deltas — the strongest single proof of correctness.
// ===========================================================================

struct WrapCase {
    uint32_t start;
    uint32_t delta;
};

class ElapsedProperty : public ::testing::TestWithParam<WrapCase> {};

TEST_P(ElapsedProperty, RecoversDeltaAcrossAnyWrap) {
    const WrapCase p = GetParam();
    const uint32_t now = static_cast<uint32_t>(p.start + p.delta);  // may wrap
    EXPECT_EQ(time_util_calc_ticks_elapsed(p.start, now), p.delta)
        << "start=0x" << std::hex << p.start
        << " delta=0x" << p.delta
        << " now=0x" << now;
}

INSTANTIATE_TEST_SUITE_P(
    AllStartsAndDeltas, ElapsedProperty,
    ::testing::ValuesIn(std::vector<WrapCase>{
        // start points: zero, small, both signed-midpoints, near-top, top
        // crossed with deltas: zero, one, small, both midpoints, near-full, full-1
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000001u},
        {0x00000000u, 0x00000064u}, {0x00000000u, 0x7FFFFFFFu},
        {0x00000000u, 0x80000000u}, {0x00000000u, 0xFFFFFFFFu},
        {0x00000001u, 0xFFFFFFFFu}, {0x00000001u, 0xFFFFFFFEu},
        {0x7FFFFFFFu, 0x00000001u}, {0x7FFFFFFFu, 0x80000001u},
        {0x80000000u, 0x7FFFFFFFu}, {0x80000000u, 0x80000000u},
        {0xFFFFFFF0u, 0x00000020u}, {0xFFFFFFF0u, 0xFFFFFFFFu},
        {0xFFFFFFFFu, 0x00000001u}, {0xFFFFFFFFu, 0xFFFFFFFFu},
    }));

// ===========================================================================
// Exhaustive-ish sweep near the wrap boundary.
// Walks start values in [MAX-8, MAX] and small deltas, confirming each lands
// on the expected modular result. Cheap, and catches off-by-one wrap errors.
// ===========================================================================

TEST(CalcTicksElapsed, DenseSweepAroundBoundary) {
    for (uint32_t back = 0; back <= 8u; ++back) {
        const uint32_t start = UINT32_MAX - back;
        for (uint32_t delta = 0; delta <= 16u; ++delta) {
            const uint32_t now = static_cast<uint32_t>(start + delta);
            EXPECT_EQ(time_util_calc_ticks_elapsed(start, now), delta)
                << "start=" << start << " delta=" << delta << " now=" << now;
        }
    }
}

}  // namespace
