#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

namespace hl
{

    // Approximate float comparison tolerance used throughout the test suite.
    inline constexpr float k_FloatEpsilon = 1e-5f;

    /** @brief Checks that two Vec2f values are approximately equal component-wise. */
    inline void RequireApproxEqual(const Vec2f& a_Lhs, const Vec2f& a_Rhs)
    {
        REQUIRE(a_Lhs[0] == Catch::Approx(a_Rhs[0]).epsilon(k_FloatEpsilon));
        REQUIRE(a_Lhs[1] == Catch::Approx(a_Rhs[1]).epsilon(k_FloatEpsilon));
    }

    /** @brief Checks that two Vec3f values are approximately equal component-wise. */
    inline void RequireApproxEqual(const Vec3f& a_Lhs, const Vec3f& a_Rhs)
    {
        REQUIRE(a_Lhs[0] == Catch::Approx(a_Rhs[0]).epsilon(k_FloatEpsilon));
        REQUIRE(a_Lhs[1] == Catch::Approx(a_Rhs[1]).epsilon(k_FloatEpsilon));
        REQUIRE(a_Lhs[2] == Catch::Approx(a_Rhs[2]).epsilon(k_FloatEpsilon));
    }

}