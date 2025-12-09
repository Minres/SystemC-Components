#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <cstdio>
#include <iostream>
#include <util/io-redirector.h>

TEST_CASE("io-redirector", "[io-redirector]") {
    util::IoRedirector::get().start();
    auto result1 = util::IoRedirector::get().get_output();
    printf("Some output");
    std::cout << "Some other output" << std::endl;
    auto result2 = util::IoRedirector::get().get_output();
    util::IoRedirector::get().stop();

    REQUIRE(result1 == "");
    REQUIRE(result2 == "Some outputSome other output\n");
}
