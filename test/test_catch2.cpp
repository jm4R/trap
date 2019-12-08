#include <catch2/catch.hpp>

#include <stdexcept>

TEST_CASE("My test 1", "all passed")
{
    CHECK(true);
    REQUIRE(true);
    CHECK_FALSE(false);
    REQUIRE_FALSE(false);
};

TEST_CASE("My test 2", "all checks failed")
{
    CHECK(false);
    CHECK_FALSE(true);
    CHECK_NOTHROW(throw "non-std exception");
    CHECK_NOTHROW(throw std::runtime_error{"runtime error message"});
};
