#pragma once

#include <cassert>
#include <experimental/source_location>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace trap {

namespace internal {

    using location = std::experimental::source_location;

    struct test_case_result {
        struct failure {
            std::string what;
            location loc;
        };

        std::vector<failure> failures;
    };

    struct test_case {
        test_case_result result;

        location loc;
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

        unsigned passed_tests{ 0 };
        unsigned failed_tests{ 0 };
        unsigned passed_assertions{ 0 };
        unsigned failed_assertions{ 0 };
    };

    struct test_token {
    };

    struct interupt_test_case {
    };

    inline test_registry global_test_registry;

    inline void handle_assertion_passed(location loc)
    {
        (void)loc;
        global_test_registry.passed_assertions++;
    }
    inline void handle_check_failed(const char* what, location loc)
    {
        global_test_registry.failed_assertions++;
        auto failure = test_case_result::failure{ what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
    }
    inline void handle_require_failed(const char* what, location loc)
    {
        global_test_registry.failed_assertions++;
        auto failure = test_case_result::failure{ what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
        throw interupt_test_case{};
    }
    inline void handle_check_noexcept_failed(location loc, std::exception& ex) {
        global_test_registry.failed_assertions++;
        auto what = std::string{ "CHECK_NOTHROW( <std::exception thrown> )\ndue to unexpected exception with message:\n  " } + ex.what();
        auto failure = test_case_result::failure{ what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
    }
    inline void handle_require_noexcept_failed(location loc, std::exception& ex) {
        global_test_registry.failed_assertions++;
        auto what = std::string{ "REQUIRE_NOTHROW( <std::exception thrown> )\ndue to unexpected exception with message:\n  " } + ex.what();
        auto failure = test_case_result::failure{ what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
        throw interupt_test_case{};
    }
    inline void handle_check_nothrow_failed(location loc) {
        global_test_registry.failed_assertions++;
        auto what = "CHECK_NOTHROW( <non-std exception thrown> )\ndue to unexpected exception with message:\n  non-std exception";
        auto failure = test_case_result::failure{ what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
    }
    inline void handle_require_nothrow_failed(location loc) {
        global_test_registry.failed_assertions++;
        auto what = "REQUIRE_NOTHROW( <non-std exception thrown> )\ndue to unexpected exception with message:\n  non-std exception";
        auto failure = test_case_result::failure{ what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
        throw interupt_test_case{};
    }

    inline void handle_test_case_passed()
    {
        global_test_registry.passed_tests++;
    }
    inline void handle_test_case_failed()
    {
        global_test_registry.failed_tests++;
    }
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

inline void test_case(const char* name, raw_function fun, internal::location loc = internal::location::current())
{
    using namespace internal;
    auto& newCase = global_test_registry.current_test_entity->cases.emplace_back();
    newCase.loc = loc;
    newCase.name = name;
    newCase.before = [] {}; //TODO: support before
    newCase.test = fun;
    newCase.cleanup = [] {}; //TODO: support cleanup
}

// ####################################################################### ASSERTIONS:

inline void check(bool value, internal::location loc = internal::location::current())
{
    using namespace internal;
    if (value)
        handle_assertion_passed(loc);
    else
        handle_check_failed("CHECK( false )", loc);
}

inline void require(bool value, internal::location loc = internal::location::current())
{
    using namespace internal;
    if (value)
        handle_assertion_passed(loc);
    else
        handle_require_failed("REQUIRE( false )", loc);
}

inline void check_false(bool value, internal::location loc = internal::location::current())
{
    using namespace internal;
    if (!value)
        handle_assertion_passed(loc);
    else
        handle_check_failed("CHECK_FALSE( true )", loc);
}

inline void require_false(bool value, internal::location loc = internal::location::current())
{
    using namespace internal;
    if (!value)
        handle_assertion_passed(loc);
    else
        handle_require_failed("REQUIRE_FALSE( true )", loc);
}

inline void check_nothrow(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    using namespace internal;
    try {
        fun();
        handle_assertion_passed(loc);
    } catch (std::exception& ex) {
        handle_check_noexcept_failed(loc, ex);
    } catch (...) {
        handle_check_nothrow_failed(loc);
    }
}

inline void require_nothrow(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    using namespace internal;
    try {
        fun();
        handle_assertion_passed(loc);
    } catch (std::exception& ex) {
        handle_require_noexcept_failed(loc, ex);
    } catch (...) {
        handle_require_nothrow_failed(loc);
    }
}

// ####################################################################### SESSION:

class session {
    //TODO: session should copy test_registry instead of copying global one
public:
    int apply_command_line(int argc, char* argv[])
    {
        // TODO: handle argc, argv
        (void)argc;
        (void)argv;
        return 0;
    }

    int run(int argc, char* argv[])
    {
        int res = apply_command_line(argc, argv);
        if (res == 0)
            res = run();
        return res;
    }

    int run()
    {
        using namespace internal;
        begin_all();
        // TODO: handle exceptions
        for (auto& e : global_test_registry.tests) {
            bool any_failed = false;
            global_test_registry.current_test_entity = &e;
            e.before_all();
            begin_entity(e.name);
            for (auto& t : e.cases) {
                global_test_registry.current_test_case = &t;
                t.before();
                try {
                    t.test();
                } catch (internal::interupt_test_case&) {
                    // ignore
                }
                t.cleanup();

                if (t.result.failures.empty())
                    handle_test_case_passed();
                else
                    handle_test_case_failed();

                print_test_case_result(any_failed);
                any_failed &= !t.result.failures.empty();
            }
        }

        end_all();

        return 2;
    }

private:
    constexpr static auto SEPARATOR1 = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    constexpr static auto SEPARATOR2 = "-------------------------------------------------------------------------------\n";
    constexpr static auto SEPARATOR3 = "...............................................................................\n\n";
    constexpr static auto SEPARATOR4 = "===============================================================================\n";

    std::ostream& out()
    {
        return std::cout;
    }

    void begin_all()
    {
        auto begin_msg = "test is a trap v0.9.1 host application.\n"
                         "Run with -? for options\n"
                         "\n";
        out() << SEPARATOR1 << begin_msg;
    }

    void end_all()
    {
        const auto& reg = internal::global_test_registry;
        out() << SEPARATOR4;

        if (reg.failed_assertions == 0) {
            out() << "All tests passed (" << reg.passed_assertions << " assertions in " << reg.passed_tests << " test_case)\n";
            return;
        }

        out() << "test cases: " << reg.failed_tests + reg.passed_tests << " | " << reg.passed_tests << " passed | " << reg.failed_tests << " failed\n";
        out() << "assertions: " << reg.failed_assertions + reg.passed_assertions << " | " << reg.passed_assertions << " passed | " << reg.failed_assertions << " failed\n";
    }

    void begin_entity(const char* name)
    {
        (void)name;
    }

    void print_test_case_result(bool already_failed_cases)
    {
        using namespace internal;

        auto* test = global_test_registry.current_test_case;
        assert(test);
        auto& result = test->result;

        if (result.failures.empty())
            return;

        if (!already_failed_cases)
            out() << SEPARATOR2 << global_test_registry.current_test_entity->name << '\n'
                  << SEPARATOR2;

        out() << test->loc.file_name() << ':' << test->loc.line() << '\n'
              << SEPARATOR3;

        for (const auto& f : result.failures) {
            out() << f.loc.file_name() << ':' << f.loc.line() << ": FAILED:\n";
            out() << "  " << f.what << "\n\n";
        }
    }
};

} // namespace trap
