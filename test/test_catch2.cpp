#include <catch2/catch.hpp>

#include <stdexcept>

TEST_CASE("My test 1", "all passed")
{
    CHECK(true);
    REQUIRE(true);
    CHECK_FALSE(false);
    REQUIRE_FALSE(false);
    CHECK_NOTHROW([] {}());
    REQUIRE_NOTHROW([] {}());
    CHECK_THROWS(throw 5);
    REQUIRE_THROWS(throw 5);
    CHECK_THROWS_AS(throw 5, int);
    REQUIRE_THROWS_AS(throw 5, int);
};

TEST_CASE("My test 2", "all checks failed")
{
    CHECK(false);
    CHECK_FALSE(true);
    CHECK_NOTHROW(throw "non-std exception");
    CHECK_NOTHROW(throw std::runtime_error{ "runtime error message" });
    CHECK_THROWS([] {}());
    CHECK_THROWS_AS(throw 5, float);
    CHECK_THROWS_AS(throw std::runtime_error{ "runtime error message" }, int);
    CHECK_THROWS_AS([] {}(), int);
};
