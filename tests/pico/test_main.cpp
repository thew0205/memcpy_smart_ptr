#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include <stdio.h>

#include "pico/stdlib.h"
int main(int ac, char **av)
{
    stdio_init_all();

    printf("Running CppUTest...");

    const char *fav[3] = {"memcpy_smart_ptr", "-c", "-v"};

    return CommandLineTestRunner::RunAllTests(3, fav);
}
