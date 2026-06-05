#include <gtest/gtest.h>
#include "../resolver/VersionConstraint.h"

namespace flyp::test {

// ── Helper ────────────────────────────────────────────────────────────────────

static bool matches(const std::string& range_str, const std::string& version) {
    return SemVerRange::parse(range_str).matches(SemVer::parse(version));
}

// ── Wildcard / any ────────────────────────────────────────────────────────────

TEST(SemVerRangeTest, AnyMatchesAll) {
    EXPECT_TRUE(matches("*",   "1.0.0"));
    EXPECT_TRUE(matches("*",   "0.0.1"));
    EXPECT_TRUE(matches("*",   "99.99.99"));
    EXPECT_TRUE(matches("",    "1.0.0"));
}

// ── Exact ─────────────────────────────────────────────────────────────────────

TEST(SemVerRangeTest, ExactMatch) {
    EXPECT_TRUE(matches("1.2.3",  "1.2.3"));
    EXPECT_FALSE(matches("1.2.3", "1.2.4"));
    EXPECT_FALSE(matches("1.2.3", "1.2.2"));
    EXPECT_FALSE(matches("1.2.3", "2.0.0"));
}

TEST(SemVerRangeTest, ExactWithLeadingV) {
    EXPECT_TRUE(matches("v1.2.3", "1.2.3"));
    EXPECT_TRUE(matches("v1.2.3", "v1.2.3"));
}

// ── Caret (^) ─────────────────────────────────────────────────────────────────

TEST(SemVerRangeTest, CaretMajorNonZero) {
    // ^1.2.3 → >=1.2.3, <2.0.0
    EXPECT_TRUE(matches("^1.2.3",  "1.2.3"));
    EXPECT_TRUE(matches("^1.2.3",  "1.2.4"));
    EXPECT_TRUE(matches("^1.2.3",  "1.9.9"));
    EXPECT_FALSE(matches("^1.2.3", "1.2.2"));
    EXPECT_FALSE(matches("^1.2.3", "2.0.0"));
    EXPECT_FALSE(matches("^1.2.3", "0.9.9"));
}

TEST(SemVerRangeTest, CaretMajorZeroMinorNonZero) {
    // ^0.2.3 → >=0.2.3, <0.3.0
    EXPECT_TRUE(matches("^0.2.3",  "0.2.3"));
    EXPECT_TRUE(matches("^0.2.3",  "0.2.9"));
    EXPECT_FALSE(matches("^0.2.3", "0.3.0"));
    EXPECT_FALSE(matches("^0.2.3", "0.2.2"));
    EXPECT_FALSE(matches("^0.2.3", "1.0.0"));
}

TEST(SemVerRangeTest, CaretMajorZeroMinorZero) {
    // ^0.0.3 → >=0.0.3, <0.0.4
    EXPECT_TRUE(matches("^0.0.3",  "0.0.3"));
    EXPECT_FALSE(matches("^0.0.3", "0.0.4"));
    EXPECT_FALSE(matches("^0.0.3", "0.0.2"));
}

// ── Tilde (~) ─────────────────────────────────────────────────────────────────

TEST(SemVerRangeTest, TildeFullVersion) {
    // ~1.2.3 → >=1.2.3, <1.3.0
    EXPECT_TRUE(matches("~1.2.3",  "1.2.3"));
    EXPECT_TRUE(matches("~1.2.3",  "1.2.9"));
    EXPECT_FALSE(matches("~1.2.3", "1.3.0"));
    EXPECT_FALSE(matches("~1.2.3", "1.2.2"));
    EXPECT_FALSE(matches("~1.2.3", "2.0.0"));
}

TEST(SemVerRangeTest, TildeMinorOnly) {
    // ~1.2 → >=1.2.0, <1.3.0
    EXPECT_TRUE(matches("~1.2",  "1.2.0"));
    EXPECT_TRUE(matches("~1.2",  "1.2.9"));
    EXPECT_FALSE(matches("~1.2", "1.3.0"));
    EXPECT_FALSE(matches("~1.2", "1.1.9"));
}

TEST(SemVerRangeTest, TildeMajorOnly) {
    // ~1 → >=1.0.0, <2.0.0
    EXPECT_TRUE(matches("~1",  "1.0.0"));
    EXPECT_TRUE(matches("~1",  "1.9.9"));
    EXPECT_FALSE(matches("~1", "2.0.0"));
    EXPECT_FALSE(matches("~1", "0.9.9"));
}

// ── Comparison operators ──────────────────────────────────────────────────────

TEST(SemVerRangeTest, GreaterThanOrEqual) {
    EXPECT_TRUE(matches(">=1.2.0",  "1.2.0"));
    EXPECT_TRUE(matches(">=1.2.0",  "1.2.1"));
    EXPECT_TRUE(matches(">=1.2.0",  "2.0.0"));
    EXPECT_FALSE(matches(">=1.2.0", "1.1.9"));
    EXPECT_FALSE(matches(">=1.2.0", "0.9.9"));
}

TEST(SemVerRangeTest, GreaterThan) {
    EXPECT_TRUE(matches(">1.2.0",  "1.2.1"));
    EXPECT_TRUE(matches(">1.2.0",  "2.0.0"));
    EXPECT_FALSE(matches(">1.2.0", "1.2.0")); // strict
    EXPECT_FALSE(matches(">1.2.0", "1.1.9"));
}

TEST(SemVerRangeTest, LessThanOrEqual) {
    EXPECT_TRUE(matches("<=2.0.0",  "2.0.0"));
    EXPECT_TRUE(matches("<=2.0.0",  "1.9.9"));
    EXPECT_FALSE(matches("<=2.0.0", "2.0.1"));
}

TEST(SemVerRangeTest, LessThan) {
    EXPECT_TRUE(matches("<2.0.0",  "1.9.9"));
    EXPECT_FALSE(matches("<2.0.0", "2.0.0")); // strict
    EXPECT_FALSE(matches("<2.0.0", "2.0.1"));
}

// ── Combined (comma = AND) ────────────────────────────────────────────────────

TEST(SemVerRangeTest, CombinedRange) {
    EXPECT_TRUE(matches(">=1.0.0,<2.0.0",  "1.5.0"));
    EXPECT_TRUE(matches(">=1.0.0,<2.0.0",  "1.0.0"));
    EXPECT_FALSE(matches(">=1.0.0,<2.0.0", "2.0.0"));
    EXPECT_FALSE(matches(">=1.0.0,<2.0.0", "0.9.9"));
}

TEST(SemVerRangeTest, CombinedRangeWithSpaces) {
    EXPECT_TRUE(matches(">=1.0.0, <2.0.0", "1.5.0"));
    EXPECT_FALSE(matches(">=1.0.0, <2.0.0", "2.0.0"));
}

TEST(SemVerRangeTest, ThreeConstraints) {
    // Three AND-ed constraints.
    EXPECT_TRUE(matches(">=1.0.0,<2.0.0,<1.9.0", "1.5.0"));
    EXPECT_FALSE(matches(">=1.0.0,<2.0.0,<1.9.0", "1.9.0")); // fails the third
    EXPECT_FALSE(matches(">=1.0.0,<2.0.0,<1.9.0", "2.0.0")); // fails the second
    EXPECT_TRUE(matches(">=1.0.0,<=1.9.9", "1.9.9"));
    EXPECT_FALSE(matches(">=1.0.0,<=1.9.9", "2.0.0"));
}

// ── Wildcard shorthand ────────────────────────────────────────────────────────

TEST(SemVerRangeTest, WildcardShorthandX) {
    // 1.x → >=1.0.0, <2.0.0
    EXPECT_TRUE(matches("1.x",  "1.0.0"));
    EXPECT_TRUE(matches("1.x",  "1.9.9"));
    EXPECT_FALSE(matches("1.x", "2.0.0"));
    EXPECT_FALSE(matches("1.x", "0.9.9"));
}

TEST(SemVerRangeTest, WildcardShorthandStar) {
    EXPECT_TRUE(matches("1.*",  "1.5.0"));
    EXPECT_FALSE(matches("1.*", "2.0.0"));
}

// ── is_any ────────────────────────────────────────────────────────────────────

TEST(SemVerRangeTest, IsAny) {
    EXPECT_TRUE(SemVerRange::parse("*").is_any());
    EXPECT_TRUE(SemVerRange::parse("").is_any());
    EXPECT_FALSE(SemVerRange::parse("^1.0.0").is_any());
    EXPECT_FALSE(SemVerRange::parse(">=1.0.0").is_any());
}

// ── Edge cases ────────────────────────────────────────────────────────────────

TEST(SemVerRangeTest, ZeroVersions) {
    EXPECT_TRUE(matches(">=0.0.0",  "0.0.0"));
    EXPECT_TRUE(matches(">=0.0.0",  "1.0.0"));
    EXPECT_TRUE(matches("^0.0.1",   "0.0.1"));
    EXPECT_FALSE(matches("^0.0.1",  "0.0.2"));
}

TEST(SemVerRangeTest, LargeVersionNumbers) {
    EXPECT_TRUE(matches("^10.20.30", "10.20.30"));
    EXPECT_TRUE(matches("^10.20.30", "10.99.0"));
    EXPECT_FALSE(matches("^10.20.30", "11.0.0"));
}

} // namespace flyp::test
