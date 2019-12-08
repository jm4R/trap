#include <catch2/catch.hpp>

TEST_CASE("My test 1", "all passed")
{
    CHECK(true);
    REQUIRE(true);
    CHECK_FALSE(true);
    REQUIRE_FALSE(true);
};

TEST_CASE("My test 2", "My case 2")
{
    CHECK(false);
    REQUIRE(false);
};
