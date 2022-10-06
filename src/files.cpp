#include "files.hpp"
#include "arguments.hpp"
#include "main.hpp"
#include "verbose.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace Files = PT::Main::Files;
namespace fs = std::filesystem;
using namespace PT;

void Files::CheckDir() {
    Verbose::SendMessage("Checking for directory...\n");

    if(!fs::exists(Main::StoreDir)) {
        Verbose::SendMessage("Directory does not exist, creating directory...\n");
        fs::create_directory(StoreDir);
        Verbose::SendMessage("Created directory\n");
    } else {
        Verbose::SendMessage("Directory already exists, skipping creation\n");
    }
}

void Files::ListDir() {
    Verbose::SendMessage("Home Directory: ");
    Verbose::SendMessage(HomeDir.c_str());
    Verbose::SendMessage("\n");

    Verbose::SendMessage("Store Directory: ");
    Verbose::SendMessage(StoreDir.c_str());
    Verbose::SendMessage("\n");

    for(const fs::directory_entry &file : fs::directory_iterator(StoreDir)) {
        std::string path = file.path();
        path.erase(0, StoreDir.length());
        std::cout << "t: " << path << "\n";
    }

    std::cout << std::flush;
}

bool Files::CheckTemplate(std::string* file) {
    std::string path = Main::StoreDir + *file + ".pt";

    return fs::exists(path) && !fs::is_directory(path);
}

bool Files::CurrDirectory() { return fs::exists(Arguments::vm["new"].as<std::string>()); }

void Files::MoveToStore() { fs::rename("temp.pt", StoreDir + Arguments::vm["create"].as<std::string>() + ".pt"); }

void Files::CopyToDir() { fs::copy(StoreDir + Arguments::vm["type"].as<std::string>() + ".pt", "."); }

void Files::Delete(std::string* path) {
    fs::remove(*path);

    if(fs::is_directory(*path)) {
        for(const fs::directory_entry &it : fs::recursive_directory_iterator(*path)) {
            fs::remove(it);
        }
    }
}

static int ExtractCopyData(struct archive* in, struct archive* out) {
    int r;
    const void* buff;
    size_t size;
    la_int64_t offset;

    for(;;) {
        r = archive_read_data_block(in, &buff, &size, &offset);

        if(r == ARCHIVE_EOF) { return (ARCHIVE_OK); }

        if(r < ARCHIVE_OK) { return (r); }

        r = archive_write_data_block(out, buff, size, offset);

        if(r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(out));

            return r;
        }
    }
}

void Files::CompressArchive() {
    std::vector<std::string> paths;
    std::string tempPath;

    for(const std::filesystem::directory_entry &file : fs::recursive_directory_iterator(".")) {
        tempPath = file.path();
        tempPath.erase(0, 2);
        paths.push_back(tempPath);
    }

    struct archive* a;
    struct archive_entry* entry;
    struct stat st;
    char buffer[8192];
    int len;
    int fd;
    FILE* f;
    static time_t now;

    a = archive_write_new();
    time(&now);

    if(Arguments::vm.count("bzip")) {
        archive_write_add_filter_bzip2(a);
    } else if(Arguments::vm.count("xz")) {
        archive_write_add_filter_xz(a);
    } else {
        archive_write_add_filter_gzip(a);
    }

    archive_write_set_format_gnutar(a);
    archive_write_open_filename(a, "temp.pt");

    for(unsigned long i = 0; i < paths.size(); i++) {
        const char* name = paths[i].c_str();

        lstat(name, &st);

        entry = archive_entry_new();

        archive_entry_set_pathname_utf8(entry, name);
        archive_entry_set_size(entry, st.st_size);
        archive_entry_set_filetype(entry, st.st_mode);
        archive_entry_set_perm(entry, 0755);

        archive_entry_set_atime(entry, now + 3, 0);
        archive_entry_set_ctime(entry, now + 5, 0);
        archive_entry_set_mtime(entry, now, 0);

        archive_write_header(a, entry);

        f = fopen(paths[i].c_str(), "r");
        fd = fileno(f);
        len = read(fd, buffer, sizeof(buffer));

        while(len > 0) {
            archive_write_data(a, buffer, sizeof(buffer));
            len = read(fd, buffer, sizeof(buffer));
        }

        close(fd);
        fclose(f);

        archive_entry_free(entry);
    }

    archive_write_free(a);
}

void Files::ExtractArchive() {
    struct archive* a;
    struct archive* out;
    struct archive_entry* entry;
    int flags;
    int r;

    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    out = archive_write_disk_new();
    archive_write_disk_set_options(out, flags);
    archive_write_disk_set_standard_lookup(out);

    if((r = archive_read_open_filename(a, (Arguments::vm["type"].as<std::string>() + ".pt").c_str(), 10240))) { exit(1); }

    const std::string destination = Arguments::vm["new"].as<std::string>();

    for(;;) {
        r = archive_read_next_header(a, &entry);

        if(r == ARCHIVE_EOF) { break; }

        if(r < ARCHIVE_OK) { fprintf(stderr, "%s\n", archive_error_string(a)); }

        if(r < ARCHIVE_WARN) { exit(1); }

        const char* currentFile = archive_entry_pathname(entry);
        const std::string fullOutputPath = destination + "/" + currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());

        r = archive_write_header(out, entry);

        if(r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(out));
        } else if(archive_entry_size(entry) > 0) {
            r = ExtractCopyData(a, out);
            if(r < ARCHIVE_OK) { fprintf(stderr, "%s\n", archive_error_string(out)); }

            if(r < ARCHIVE_WARN) { exit(1); }
        }

        r = archive_write_finish_entry(out);

        if(r < ARCHIVE_OK) { fprintf(stderr, "%s\n", archive_error_string(out)); }

        if(r < ARCHIVE_WARN) { exit(1); }
    }

    archive_read_free(a);
    archive_write_free(out);
}