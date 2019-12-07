#include <catch2/catch.hpp>

TEST_CASE("My test 1", "My case 1")
{
    CHECK(true);
    REQUIRE(true);
};

TEST_CASE("My test 2", "My case 2")
{
    CHECK(false);
    REQUIRE(false);
};
