#include "main.hpp"
#include "arguments.hpp"
#include "files.hpp"
#include <iostream>
#include <string>

namespace Arguments = PT::Main::Arguments;
namespace Files = PT::Main::Files;
using namespace PT;

int Main::argc;
const char** Main::argv;
std::string Main::HomeDir;
std::string Main::StoreDir;

int main(int argc, const char* argv[]) {
    if(argc == 1) {
        printf("No options specified! Use %s --help to view the help menu\n", argv[0]);
        return 1;
    }

    Main::argc = argc;
    Main::argv = argv;

    if((Main::HomeDir = getenv("HOME")).c_str() == nullptr) { Main::HomeDir = getpwuid(getuid())->pw_dir; }
    Main::StoreDir = Main::HomeDir + "/.local/share/pt/";

    Files::CheckDir();
    Arguments::CollectArgs();
    Arguments::ExecArgs();
}