#include "platform/x11/window.hpp"

#include <exception>
#include <iostream>

int main()
{
    try {
        cpu_renderer::x11::runWindow();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "cpu-renderer: " << error.what() << '\n';
        return 1;
    }
}
