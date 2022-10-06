#include "verbose.hpp"
#include "arguments.hpp"
#include <iostream>

namespace Verbose = PT::Main::Verbose;
using namespace PT;

void Verbose::SendMessage(const char* str) {
    if(Arguments::IsVerbose) {
        std::cout << str;
    }
}