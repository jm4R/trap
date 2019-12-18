#pragma once

#include <cassert>
#include <experimental/source_location>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace trap {

namespace internal {

    using namespace std::literals::string_literals;
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

    template <typename T>
    struct test_holder {
        test_holder() { instance.test(); }
        test_holder(const test_holder&) = delete;
        test_holder(test_holder&&) = delete;
        test_holder& operator=(const test_holder&) = delete;
        test_holder& operator=(test_holder&&) = delete;
        T instance{};
    };

    struct interupt_test_case {
    };

    inline test_registry global_test_registry;

    enum assertion_type {
        at_check,
        at_require
    };

    inline void fail(std::string&& what, const location& loc, assertion_type t)
    {
        global_test_registry.failed_assertions++;
        const char* prefix = (t == at_check) ? "CHECK" : "REQUIRE";
        auto failure = test_case_result::failure{ prefix + what, loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
        if (t == at_require)
            throw interupt_test_case{};
    }

    inline void handle_assertion_passed(location loc)
    {
        (void)loc;
        global_test_registry.passed_assertions++;
    }
    inline void handle_noexcept_failed(location loc, const std::exception& ex, assertion_type t)
    {
        fail("_NOTHROW( <std::exception thrown> )\ndue to unexpected exception with message:\n  "s + ex.what(), loc, t);
    }
    inline void handle_nothrow_failed(location loc, assertion_type t)
    {
        fail("_NOTHROW( <non-std exception thrown> )\ndue to unexpected exception with message:\n  non-std exception", loc, t);
    }
    inline void handle_throws_failed(location loc, assertion_type t)
    {
        fail("_THROWS( <no exception thrown> )\nbecause no exception was thrown where one was expected", loc, t);
    }
    inline void handle_throws_as_failed(location loc, assertion_type t)
    {
        fail("_THROWS_AS( <no exception thrown> )\nbecause no exception was thrown where one was expected", loc, t);
    }
    inline void handle_throws_as_type_failed(location loc, assertion_type t)
    {
        fail("_THROWS_AS( <exception thrown> )\ndue to unexpected exception with message:\n  Unknown exception", loc, t);
    }
    inline void handle_throws_as_type_failed(location loc, const std::exception& ex, assertion_type t)
    {
        fail("_THROWS_AS( <exception thrown> )\ndue to unexpected exception with message:\n  "s + ex.what(), loc, t);
    }
    inline void handle_throws_with_failed(location loc, assertion_type t)
    {
        fail("_THROWS_WITH( <no exception thrown> )\nbecause no exception was thrown where one was expected\n", loc, t);
    }
    inline void handle_throws_with_failed(location loc, const std::string& expected, assertion_type t)
    {
        fail("_THROWS_WITH( <exception thrown> )\nwith expansion:\n  \"Unknown exception\" equals: \"" + expected + '\"', loc, t);
    }
    inline void handle_throws_with_failed(location loc, const std::exception& ex, const std::string& expected, assertion_type t)
    {
        fail("_THROWS_WITH( <exception thrown> )\nwith expansion:\n  \""s + ex.what() + "\" equals: \"" + expected + '\"', loc, t);
    }

    inline void handle_test_case_passed()
    {
        global_test_registry.passed_tests++;
    }
    inline void handle_test_case_failed()
    {
        global_test_registry.failed_tests++;
    }

    //######################################################################## ASSERTIONS:
    inline void as_true(bool value, internal::location loc, assertion_type t)
    {
        if (value)
            handle_assertion_passed(loc);
        else
            fail("( false )", loc, t);
    }

    inline void as_false(bool value, internal::location loc, assertion_type t)
    {
        if (!value)
            handle_assertion_passed(loc);
        else
            fail("_FALSE( true )\nwith expansion:\n  !true", loc, t); //save it as a "funny" easter egg and for compatibility with Catch2
    }

    inline void as_nothrow(std::function<void()>&& fun, internal::location loc, assertion_type t)
    {
        try {
            fun();
            handle_assertion_passed(loc);
        } catch (std::exception& ex) {
            handle_noexcept_failed(loc, ex, t);
        } catch (...) {
            handle_nothrow_failed(loc, t);
        }
    }

    inline void as_throws(std::function<void()>&& fun, internal::location loc, assertion_type t)
    {
        try {
            fun();
            handle_throws_failed(loc, t);
        } catch (interupt_test_case&) {
            throw;
        } catch (...) {
            handle_assertion_passed(loc);
        }
    }

    template <typename Ex>
    inline void as_throws_as(std::function<void()>&& fun, internal::location loc, assertion_type t)
    {
        try {
            fun();
            handle_throws_as_failed(loc, t);
        } catch (interupt_test_case&) {
            throw;
        } catch (const Ex&) {
            handle_assertion_passed(loc);
        } catch (std::exception& ex) {
            handle_throws_as_type_failed(loc, ex, t);
        } catch (...) {
            handle_throws_as_type_failed(loc, t);
        }
    }

    inline void as_throws_with(std::function<void()>&& fun, const std::string& expected, internal::location loc, assertion_type t)
    {
        try {
            fun();
            handle_throws_with_failed(loc, t);
        } catch (interupt_test_case&) {
            throw;
        } catch (std::exception& ex) {
            if (ex.what() == expected) {
                handle_assertion_passed(loc);
                return;
            }
            handle_throws_with_failed(loc, ex, expected, t);
        } catch (...) {
            handle_throws_with_failed(loc, expected, t);
        }
    }
} //namespace internal

template <typename T>
inline internal::test_holder<T> test_register(const char* name)
{
    using namespace internal;
    global_test_registry.current_test_entity = &global_test_registry.tests.emplace_back();
    global_test_registry.current_test_entity->name = name;
    global_test_registry.current_test_entity->before_all = [] { /*TODO*/ };
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
    internal::as_true(value, loc, internal::at_check);
}
inline void require(bool value, internal::location loc = internal::location::current())
{
    internal::as_true(value, loc, internal::at_require);
}
inline void check_false(bool value, internal::location loc = internal::location::current())
{
    internal::as_false(value, loc, internal::at_check);
}
inline void require_false(bool value, internal::location loc = internal::location::current())
{
    internal::as_false(value, loc, internal::at_require);
}
inline void check_nothrow(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    internal::as_nothrow(std::move(fun), loc, internal::at_check);
}
inline void require_nothrow(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    internal::as_nothrow(std::move(fun), loc, internal::at_require);
}
inline void check_throws(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    internal::as_throws(std::move(fun), loc, internal::at_check);
}
inline void require_throws(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    internal::as_throws(std::move(fun), loc, internal::at_require);
}
template <typename Ex>
inline void check_throws_as(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    internal::as_throws_as<Ex>(std::move(fun), loc, internal::at_check);
}
template <typename Ex>
inline void require_throws_as(std::function<void()>&& fun, internal::location loc = internal::location::current())
{
    internal::as_throws_as<Ex>(std::move(fun), loc, internal::at_require);
}
inline void check_throws_with(std::function<void()>&& fun, const std::string& expected, internal::location loc = internal::location::current())
{
    internal::as_throws_with(std::move(fun), expected, loc, internal::at_check);
}
inline void require_throws_with(std::function<void()>&& fun, const std::string& expected, internal::location loc = internal::location::current())
{
    internal::as_throws_with(std::move(fun), expected, loc, internal::at_require);
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
