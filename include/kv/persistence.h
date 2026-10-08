#pragma once

#include <filesystem>
#include <mutex>
#include <string>

namespace kv {
class KeyValueStore;

class Persistence {
public:
    explicit Persistence(std::filesystem::path file = "data/store.snapshot");

    bool save(const KeyValueStore& store);
    bool load(KeyValueStore& store);
    const std::filesystem::path& file() const { return file_; }

private:
    std::filesystem::path file_;
    mutable std::mutex mutex_;
};
} // namespace kv
