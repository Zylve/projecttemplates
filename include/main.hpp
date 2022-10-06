#pragma once
#include <cstdlib>
#include <pwd.h>
#include <string>
#include <unistd.h>

namespace PT::Main {
    extern int argc;
    extern const char** argv;

    extern std::string HomeDir;
    extern std::string StoreDir;
} // namespace PT::Main