/*
 * main.cpp
 *
 *  Created on: 01.01.2019
 *      Author: eyck
 */

#include <cassert>
#include <cstdio>
#include <iostream>
#include <util/io-redirector.h>

using namespace util;

int main(int arcg, char* argv[]) {
    IoRedirector::get().start();
    auto result1 = IoRedirector::get().get_output();
    assert(result1 == "");
    printf("Some output");
    std::cout << "Some other output" << std::endl;
    auto result2 = IoRedirector::get().get_output();
    assert(result2 == "Some outputSome other output\n");
    IoRedirector::get().stop();
}
