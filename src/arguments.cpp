#include "arguments.hpp"
#include "files.hpp"
#include "main.hpp"
#include "verbose.hpp"
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace Arguments = PT::Main::Arguments;

opt::variables_map Arguments::vm;
bool Arguments::IsVerbose = false;
bool Arguments::ExecScript = true;
bool Arguments::Overwrite = false;

void Arguments::CollectArgs() {
    opt::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Show this menu")
        ("verbose,v", "Be verbose")
        ("skip-exec,s", "Do not execute setup.sh if it exists")
        ("list,l", "List all templates")
        ("overwrite,o", "Overwrite an existing template or project")
        ("create,c", opt::value<std::string>(), "Create a template")
        ("delete,d", opt::value<std::string>(), "Delete a template")
        ("type,t", opt::value<std::string>(), "Specify template")
        ("new,n", opt::value<std::string>(), "Specify project name")
    ;

    opt::store(
        opt::command_line_parser(Main::argc, Main::argv)
            .options(desc)
            .style(
                opt::command_line_style::unix_style
                | opt::command_line_style::allow_long_disguise
                | opt::command_line_style::allow_sticky
                | opt::command_line_style::short_allow_next
            )
        .run(),
    vm);
    // clang-format on

    opt::notify(Arguments::vm);

    if(Arguments::vm.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }
}

void Arguments::ExecArgs() {
    if(Arguments::vm.count("verbose")) { Arguments::IsVerbose = true; }
    if(Arguments::vm.count("list")) {
        Files::ListDir();
        exit(0);
    }

    if(Arguments::vm.count("skip-exec")) { Arguments::ExecScript = false; }
    if(Arguments::vm.count("overwrite")) { Arguments::Overwrite = true; }

    if(Arguments::vm.count("create")) {
        std::string path = Arguments::vm["create"].as<std::string>();

        Verbose::SendMessage("Checking if template exists\n");
        if(Files::CheckTemplate(&path) && !Overwrite) {
            std::cout << "Template already exists! Pass -o to overwrite." << std::endl;
            exit(1);
        } else {
            Verbose::SendMessage("Compressing project\n");
            Files::CompressArchive();

            Verbose::SendMessage("Moving archive\n");
            Files::MoveToStore();
        }
    }

    if(Arguments::vm.count("delete")) {
        Verbose::SendMessage("Checking if template exists\n");
        std::string path = StoreDir + vm["delete"].as<std::string>() + ".pt";

        if(Files::CheckTemplate(&path)) {
            std::cout << "Template does not exist!" << std::endl;
            exit(1);
        }

        Verbose::SendMessage("Deleting template\n");
        Files::Delete(&path);
    }

    if(Arguments::vm.count("type")) {
        std::string path = vm["type"].as<std::string>();

        if(!Arguments::vm.count("new")) {
            std::cout << "Use -n to specify a project name!" << std::endl;
            exit(1);
        }

        if(Files::CurrDirectory() && !Overwrite) {
            std::cout << "Project already exists in this directory!" << std::endl;
        } else {
            if(!Files::CheckTemplate(&path)) {
                std::cout << "Template does not exist!" << std::endl;
                exit(1);
            }

            Verbose::SendMessage("Deleting existing project\n");
            Files::Delete(&path);

            Verbose::SendMessage("Copying template\n");
            Files::CopyToDir();

            Verbose::SendMessage("Inflating template\n");
            Files::ExtractArchive();

            Verbose::SendMessage("Deleting temporary files\n");
            std::string pt = path + ".pt";
            Files::Delete(&pt);

            if(!Arguments::vm.count("skip-exec") && std::filesystem::exists(Arguments::vm["new"].as<std::string>() + "/setup.sh")) {
                Verbose::SendMessage("Executing setup.sh\n");
                std::string com = "cd " + Arguments::vm["new"].as<std::string>() + " && sh setup.sh";
                system(com.c_str());
            }
        }
    }
}