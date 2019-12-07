#pragma once

#include <cassert>
#include <experimental/source_location>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace trap {

namespace internal {

    using location = std::experimental::source_location;

    struct test_case_result {
        struct failure {
            const char* what;
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
    };

    struct test_token {
    };

    struct interupt_test_case {
    };

    inline test_registry global_test_registry;

    // TODO: implement handlers
    inline void handle_check_passed(location loc) {}
    inline void handle_check_failed(location loc)
    {
        auto failure = test_case_result::failure{ "CHECK( false )", loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
    }
    inline void handle_require_passed(location loc) {}
    inline void handle_require_failed(location loc)
    {
        auto failure = test_case_result::failure{ "REQUIRE( false )", loc };
        auto& res = global_test_registry.current_test_case->result;
        res.failures.push_back(failure);
        throw interupt_test_case{};
    }
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

class session {

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
                print_test_case_result(any_failed);
                any_failed &= !t.result.failures.empty();
            }
        }
        return 2;
    }

private:
    constexpr static auto SEPARATOR1 = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    constexpr static auto SEPARATOR2 = "-------------------------------------------------------------------------------\n";
    constexpr static auto SEPARATOR3 = "...............................................................................\n\n";

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

    void begin_entity(const char* name)
    {
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
