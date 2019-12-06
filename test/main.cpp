#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <trap/trap.hpp>

TEST_CASE("My test 1", "My case 1")
{
    CHECK(false);
    REQUIRE(false);
};

struct MyTest1 {
    void test()
    {
        trap::test_case("My case 1", []{
            trap::check(false);
            trap::require(false);
        });
    }
};

namespace {
const auto global_MyTest1 = ::trap::test_register<MyTest1>("MyTest1");
}

int main( int argc, char* argv[] ) {
    // global setup...

    int result = Catch::Session().run( argc, argv );

    // global clean-up...

    trap::run();

    return result;
}
