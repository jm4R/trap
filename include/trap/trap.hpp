#pragma once

#include <experimental/source_location>
#include <functional>
#include <memory>
#include <vector>

namespace trap {

namespace internal {

    using location = std::experimental::source_location;

    struct test_case {
        std::function<void()> before;
        std::function<void()> test;
        std::function<void()> cleanup;
        const char* name;
    };

    struct test_entity {
        std::function<void()> before_all;
        std::vector<test_case> cases;
        const char* name;
    };

    struct test_registry {
        std::vector<test_entity> tests;
        test_entity* current_test_entity{ nullptr };
        test_case* current_test_case{ nullptr };
    };

    struct test_token {
    };

    inline test_registry global_test_registry;

    // TODO: implement handlers
    inline void handle_check_passed(location loc) {}
    inline void handle_check_failed(location loc) {}
    inline void handle_require_passed(location loc) {}
    inline void handle_require_failed(location loc) {}
    inline void handle_unexpected_exception(location loc, std::exception& ex) {}
    inline void handle_unexpected_throw(location loc) {}
    inline void handle_expected_exception(location loc, std::exception& ex) {}
    inline void handle_expected_throw(location loc) {}
} //namespace internal

template <typename T>
inline internal::test_token test_register(const char* name)
{
    using namespace internal;
    auto obj = std::make_shared<T>();
    global_test_registry.current_test_entity = &global_test_registry.tests.emplace_back();
    global_test_registry.current_test_entity->name = name;
    global_test_registry.current_test_entity->before_all = [obj] { /*TODO*/ };
    obj->test();
    return {};
}

using raw_function = void (*)();

inline void test_case(const char* name, raw_function fun)
{
    using namespace internal;
    auto& newCase = global_test_registry.current_test_entity->cases.emplace_back();
    newCase.name = name;
    newCase.before = [] {}; //TODO: support before
    newCase.test = fun;
    newCase.cleanup = [] {}; //TODO: support cleanup
}

inline void check(bool value, internal::location loc = internal::location::current())
{
    using namespace internal;
    if (value)
        handle_check_passed(loc);
    else
        handle_check_failed(loc);
}

inline void require(bool value, internal::location loc = internal::location::current())
{
    using namespace internal;
    if (value)
        handle_require_passed(loc);
    else
        handle_require_failed(loc);
}

inline void run()
{
    // TODO: handle exceptions
    using namespace internal;
    for (auto& e : global_test_registry.tests) {
        global_test_registry.current_test_entity = &e;
        e.before_all();
        for (auto& t : e.cases) {
            global_test_registry.current_test_case = &t;
            t.before();
            t.test();
            t.cleanup();
        }
    }
}

} // namespace trap
