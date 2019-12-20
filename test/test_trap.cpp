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
            trap::check_throws_as<int>([] { throw 5; });
            trap::require_throws_as<double>([] { throw 5.1; });
            trap::check_throws_with([] { throw std::runtime_error{ "aaa" }; }, "aaa");
            trap::require_throws_with([] { throw std::runtime_error{ "aaa" }; }, "aaa");
        });

        trap::test_case("all checks failed", [] {
            trap::check(false);
            trap::check_false(true);
            trap::check_nothrow([] { throw "non-std exception"; });
            trap::check_nothrow([] { throw std::runtime_error{ "runtime error message" }; });
            trap::check_throws([] {});
            trap::check_throws_as<float>([] { throw 5; });
            trap::check_throws_as<int>([] { throw std::runtime_error{ "runtime error message" }; });
            trap::check_throws_as<int>([] {});
            trap::check_throws_with([] { throw 5; }, "aaa");
            trap::check_throws_with([] { throw std::runtime_error{ "aaa" }; }, "bbb");
            trap::check_throws_with([] {}, "bbb");
        });
    }
};

namespace {
const auto global_MyTest1 = ::trap::test_register<MyTest1>("My test 1");
}
