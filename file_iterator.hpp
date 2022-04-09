// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#pragma once
// file_iterator.hpp

#ifdef _MSC_VER
extern "C" {
#include "dirent.h"
}
#else
extern "C" {
#include <dirent.h> // even MINGW has it
}
#endif

#include <cstdio>
#include <string_view>

namespace my {
template <typename Cb>
[[maybe_unused]] static inline int listdir(const char* path, Cb&& callback) {
    struct dirent* entry;
    DIR* dp = opendir(path);
    if (dp == nullptr) {
        perror("opendir: Path does not exist or could not be read.");
        return -1;
    }

    while ((entry = readdir(dp)) != nullptr) { // NOLINT(concurrency-mt-unsafe)
        auto val = callback(entry);
        if (val < 0) {
            return val;
        }
    }

    closedir(dp);
    return 0;
}

template <typename Cb>
[[maybe_unused]] static inline int listdir_recursive(
    const char* name, Cb&& callback) {

    struct dirent* entry;

    DIR* dir = opendir(name);
    if (!dir) return -1;

    while ((entry = readdir(dir)) != nullptr) {
        const auto val = callback(entry);
        if (val < 0) return val;
        if (entry->d_type == DT_DIR) {
            char path[1024] = {};
            if (strcmp(entry->d_name, ".") == 0
                || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            // printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir_recursive(path, callback);
        } else {
            //  printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
    return 0;
}

[[maybe_unused]] static inline bool is_dot(const std::string_view name) {

    const int i = static_cast<int>(name.find_first_not_of('.'));
    if (i < 0) {
        return true;
    }
    return false;
}

[[maybe_unused]] static inline bool is_directory(const dirent* entry) {
    return (entry->d_type == DT_DIR);
}

} // namespace my
