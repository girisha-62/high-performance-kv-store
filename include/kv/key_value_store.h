#pragma once

#include <chrono>
#include <cstddef>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace kv {

struct Entry {
    std::string value;
    std::optional<std::chrono::steady_clock::time_point> expires_at;
};

class KeyValueStore {
public:
    explicit KeyValueStore(std::size_t shard_count = 16);

    bool set(const std::string& key, const std::string& value,
             std::optional<std::chrono::seconds> ttl = std::nullopt);
    std::optional<std::string> get(const std::string& key);
    bool exists(const std::string& key);
    bool remove(const std::string& key);
    std::size_t size();
    void clear();

    // Returns a snapshot of non-expired entries for persistence.
    std::vector<std::pair<std::string, std::string>> snapshot();

private:
    struct Shard {
        mutable std::shared_mutex mutex;
        std::unordered_map<std::string, Entry> data;
    };

    Shard& shard_for(const std::string& key);
    const Shard& shard_for(const std::string& key) const;
    bool is_expired(const Entry& entry) const;

    std::vector<Shard> shards_;
};

} // namespace kv
