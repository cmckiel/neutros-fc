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
#include <tuple>
#include <vector>

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

// GoogleTest suite for:
//     bool time_util_timed_out(uint32_t time_start,
//                              uint32_t time_now,
//                              uint32_t timeout);
//
// Contract: returns true once (time_now - time_start), computed in modulo-2^32
// arithmetic, is >= timeout. Because the elapsed delta is a *duration* (an arc
// length), the >= comparison carries no additional rollover hazard beyond the
// subtraction itself. Valid while true elapsed < 2^32 (i.e. you poll often
// enough that the counter never laps you between checks).
//
// Semantics pinned by these tests:
//   * boundary is inclusive: elapsed == timeout  -> timed out (true)
//   * timeout == 0          : timed out immediately, even at elapsed 0
//   * a naive "time_now >= time_start + timeout" implementation FAILS this
//     suite (see NearWrap_NoFalsePositive and the property test).

// ===========================================================================
// Happy path: no rollover
// ===========================================================================

TEST(TimeUtilTimedOut, NotTimedOutBeforeTimeout) {
    EXPECT_FALSE(time_util_timed_out(0u, 50u, 100u));
    EXPECT_FALSE(time_util_timed_out(1000u, 1099u, 100u));   // elapsed 99 < 100
}

TEST(TimeUtilTimedOut, TimedOutAfterTimeout) {
    EXPECT_TRUE(time_util_timed_out(0u, 150u, 100u));
    EXPECT_TRUE(time_util_timed_out(1000u, 2000u, 100u));
}

TEST(TimeUtilTimedOut, BoundaryIsInclusive) {
    EXPECT_FALSE(time_util_timed_out(0u, 99u, 100u));   // elapsed 99  -> no
    EXPECT_TRUE (time_util_timed_out(0u, 100u, 100u));  // elapsed 100 -> yes
    EXPECT_TRUE (time_util_timed_out(0u, 101u, 100u));  // elapsed 101 -> yes
}

// ===========================================================================
// Degenerate timeouts
// ===========================================================================

TEST(TimeUtilTimedOut, ZeroTimeoutFiresImmediately) {
    EXPECT_TRUE(time_util_timed_out(0u, 0u, 0u));            // elapsed 0 >= 0
    EXPECT_TRUE(time_util_timed_out(500u, 500u, 0u));
    EXPECT_TRUE(time_util_timed_out(0xFFFFFFFFu, 0xFFFFFFFFu, 0u));
}

TEST(TimeUtilTimedOut, MaxTimeoutNeedsFullRangeMinusOne) {
    // With timeout == UINT32_MAX, only elapsed == UINT32_MAX trips it.
    EXPECT_FALSE(time_util_timed_out(10u, 10u, UINT32_MAX));            // elapsed 0
    EXPECT_FALSE(time_util_timed_out(10u, 9u + 10u - 1u, UINT32_MAX)); // still short
    EXPECT_TRUE (time_util_timed_out(10u, 9u, UINT32_MAX));            // elapsed 0xFFFFFFFF
}

// ===========================================================================
// Rollover: the reason this function needs care
// ===========================================================================

// The exact scenario a naive "now >= start + timeout" gets WRONG:
// start sits 16 below the top, only 5 ticks have elapsed, timeout is 32.
// start + timeout wraps to a tiny value, so the naive form fires early.
// The correct duration-based form must report NOT timed out.
TEST(TimeUtilTimedOut, NearWrap_NoFalsePositive) {
    const uint32_t start   = 0xFFFFFFF0u;
    const uint32_t timeout = 32u;
    EXPECT_FALSE(time_util_timed_out(start, 0xFFFFFFF5u, timeout)); // elapsed 5
    EXPECT_FALSE(time_util_timed_out(start, 0x00000005u, timeout)); // elapsed 21, past wrap
    EXPECT_FALSE(time_util_timed_out(start, 0x0000000Fu, timeout)); // elapsed 31, one short
}

// Same setup, now genuinely expired across the wrap.
TEST(TimeUtilTimedOut, AcrossWrap_TrueTimeout) {
    const uint32_t start   = 0xFFFFFFF0u;
    const uint32_t timeout = 32u;
    EXPECT_TRUE(time_util_timed_out(start, 0x00000010u, timeout)); // elapsed 32 exactly
    EXPECT_TRUE(time_util_timed_out(start, 0x00000020u, timeout)); // elapsed 48
}

// A single tick that crosses MAX -> 0 with a 1-tick timeout.
TEST(TimeUtilTimedOut, SingleTickAcrossBoundary) {
    EXPECT_FALSE(time_util_timed_out(UINT32_MAX, UINT32_MAX, 1u)); // elapsed 0
    EXPECT_TRUE (time_util_timed_out(UINT32_MAX, 0u, 1u));         // elapsed 1
}

// Boundary sitting exactly on the wrap point.
TEST(TimeUtilTimedOut, BoundaryLandsOnWrap) {
    const uint32_t start = 0xFFFFFFFEu;  // 2 below top
    EXPECT_FALSE(time_util_timed_out(start, 0xFFFFFFFFu, 2u)); // elapsed 1 < 2
    EXPECT_TRUE (time_util_timed_out(start, 0x00000000u, 2u)); // elapsed 2 == 2 (at wrap)
}

// ===========================================================================
// Documented limitation: a *full* lap aliases back to "not timed out".
// This is inherent to a bare counter and is why callers must poll before
// 2^32 ticks elapse. Encoded so the boundary of validity is explicit, not a
// surprise.
// ===========================================================================

TEST(TimeUtilTimedOut, FullLapAliasesToNotTimedOut_KnownLimitation) {
    // now == start reads as elapsed 0, whether 0 or exactly 2^32 ticks passed.
    EXPECT_FALSE(time_util_timed_out(0x12345678u, 0x12345678u, 100u));
}

// ===========================================================================
// Property test: for ANY start and ANY timeout,
//     time_util_timed_out(start, start + elapsed, timeout) == (elapsed >= timeout)
// across a sweep of elapsed values (boundary neighborhood + wrap extremes).
// A correct duration-based impl satisfies this identically; a deadline-based
// (start + timeout) impl breaks it wherever the deadline wraps.
// ===========================================================================

class TimedOutProperty
    : public ::testing::TestWithParam<std::tuple<uint32_t, uint32_t>> {};

TEST_P(TimedOutProperty, MatchesElapsedGeTimeout) {
    const uint32_t start   = std::get<0>(GetParam());
    const uint32_t timeout = std::get<1>(GetParam());

    // Probes in modular space: exact boundary, its neighbors, and wrap extremes.
    const uint32_t probes[] = {
        0u, 1u, 2u,
        static_cast<uint32_t>(timeout - 1u), timeout, static_cast<uint32_t>(timeout + 1u),
        0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFEu, 0xFFFFFFFFu,
    };
    for (uint32_t elapsed : probes) {
        const uint32_t now = static_cast<uint32_t>(start + elapsed);
        const bool expected = (elapsed >= timeout);
        EXPECT_EQ(time_util_timed_out(start, now, timeout), expected)
            << "start=0x" << std::hex << start
            << " timeout=0x" << timeout
            << " elapsed=0x" << elapsed
            << " now=0x" << now;
    }
}

INSTANTIATE_TEST_SUITE_P(
    StartsCrossTimeouts, TimedOutProperty,
    ::testing::Combine(
        ::testing::Values(0x00000000u, 0x00000001u, 0x7FFFFFFFu,
                          0x80000000u, 0xFFFFFFF0u, 0xFFFFFFFFu),   // starts
        ::testing::Values(0u, 1u, 100u, 0x7FFFFFFFu,
                          0x80000000u, 0xFFFFFFFFu)));               // timeouts

// ===========================================================================
// Dense sweep straddling the wrap: starts in [MAX-8, MAX], a few timeouts,
// elapsed 0..16. Catches off-by-one errors right at the boundary.
// ===========================================================================

TEST(TimeUtilTimedOut, DenseSweepAroundBoundary) {
    for (uint32_t back = 0; back <= 8u; ++back) {
        const uint32_t start = UINT32_MAX - back;
        for (uint32_t timeout = 0; timeout <= 8u; ++timeout) {
            for (uint32_t elapsed = 0; elapsed <= 16u; ++elapsed) {
                const uint32_t now = static_cast<uint32_t>(start + elapsed);
                EXPECT_EQ(time_util_timed_out(start, now, timeout), elapsed >= timeout)
                    << "start=" << start << " timeout=" << timeout
                    << " elapsed=" << elapsed << " now=" << now;
            }
        }
    }
}

}  // namespace
