#include <trap/trap.hpp>

struct MyTest1 {
    void test()
    {
        trap::test_case("all passed", [] {
            trap::check(true);
            trap::require(true);
            trap::check_false(false);
            trap::require_false(false);
            trap::check_nothrow([] {});
            trap::require_nothrow([] {});
            trap::check_throws([] { throw 5; });
            trap::require_throws([] { throw 5; });
        });
    }
};

namespace {
const auto global_MyTest1 = ::trap::test_register<MyTest1>("My test 1");
}

struct MyTest2 {
    void test()
    {
        trap::test_case("all checks failed", [] {
            trap::check(false);
            trap::check_false(true);
            trap::check_nothrow([] { throw "non-std exception"; });
            trap::check_nothrow([] { throw std::runtime_error{ "runtime error message" }; });
            trap::check_throws([] {});
            trap::check_throws_as<float>([] { throw 5; });
            trap::check_throws_as<int>([] { throw std::runtime_error{ "runtime error message" }; });
            trap::check_throws_as<int>([] {});
        });
    }
};

namespace {
const auto global_MyTest2 = ::trap::test_register<MyTest2>("My test 2");
}
