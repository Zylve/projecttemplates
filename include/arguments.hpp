#pragma once

#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>

namespace opt = boost::program_options;

namespace PT::Main::Arguments {
    extern opt::variables_map vm;
    extern void CollectArgs();
    extern void ExecArgs();

    extern bool IsVerbose;
    extern bool ExecScript;
    extern bool Overwrite;
} // namespace PT::Main::Arguments