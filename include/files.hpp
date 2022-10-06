#pragma once

#include <string>

namespace PT::Main::Files {
    extern void CheckDir();
    extern void ListDir();

    extern bool CheckTemplate(std::string*);
    extern bool CurrDirectory();

    extern void ExtractArchive();
    extern void CompressArchive();

    extern void MoveToStore();
    extern void CopyToDir();

    extern void Delete(std::string*);
} // namespace PT::Main::Files