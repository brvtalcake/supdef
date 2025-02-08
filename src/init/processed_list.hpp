#include <detail/globals.hpp>
#include <init.hpp>
#include <types.hpp>

GLOBAL_DEF_START(std::vector<std::shared_ptr<const stdfs::path>>, already_processed_files)
    already_processed_files.reserve(32);
GLOBAL_DEF_END(std::vector<std::shared_ptr<const stdfs::path>>, already_processed_files)

GLOBAL_DEF_START(std::vector<stdfs::path>, supdef_import_paths)
    supdef_import_paths.reserve(32);
GLOBAL_DEF_END(std::vector<stdfs::path>, supdef_import_paths)