#include <trap/trap.hpp>

struct MyTest1 {
    void test()
    {
        trap::test_case("all passed", []{
            trap::check(true);
            trap::require(true);
            trap::check_false(false);
            trap::require_false(false);
        });
    }
};

namespace {
const auto global_MyTest1 = ::trap::test_register<MyTest1>("My test 1");
}

struct MyTest2 {
    void test()
    {
        trap::test_case("My case 2", []{
            trap::check(false);
            trap::require(false);
        });

        trap::test_case("My case 3", []{
            trap::check(true);
            trap::check(true);
            trap::check(true);
            trap::check(true);
            trap::check(true);
        });
    }
};

namespace {
const auto global_MyTest2 = ::trap::test_register<MyTest2>("My test 2");
}
