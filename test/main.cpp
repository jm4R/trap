#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <trap/trap.hpp>

int main(/*int argc, char* argv[]*/)
{
    const char* argv_fake[] = { "./test", "-o", "output_catch2.txt" };
    int argc = sizeof(argv_fake) / sizeof(char*);
    auto argv = const_cast<char**>(argv_fake);

    int catch_result = Catch::Session{}.run(argc, argv);
    int trap_result = trap::session{}.run(argc, argv);

    assert(catch_result == trap_result);

    return catch_result;
}
