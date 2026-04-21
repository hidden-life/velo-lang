#include <iostream>

#ifndef VELO_VERSION
#define VELO_VERSION "dev"
#endif

int main() {
    std::cout << "velo " << VELO_VERSION << std::endl;
    std::cout << "Velo bootstrap is ready." << std::endl;
    std::cout << "Next milestone: source, diagnostics, lexer." << std::endl;

    return EXIT_SUCCESS;
}