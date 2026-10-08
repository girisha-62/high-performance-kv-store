#include "kv/key_value_store.h"

#include <functional>

namespace kv {

KeyValueStore::KeyValueStore(std::size_t shard_count)
    : shards_(shard_count == 0 ? 1 : shard_count) {}

KeyValueStore::Shard& KeyValueStore::shard_for(const std::string& key) {
    return shards_[std::hash<std::string>{}(key) % shards_.size()];
}

const KeyValueStore::Shard& KeyValueStore::shard_for(const std::string& key) const {
    return shards_[std::hash<std::string>{}(key) % shards_.size()];
}

bool KeyValueStore::is_expired(const Entry& entry) const {
    return entry.expires_at && std::chrono::steady_clock::now() >= *entry.expires_at;
}

bool KeyValueStore::set(const std::string& key, const std::string& value,
                        std::optional<std::chrono::seconds> ttl) {
    if (key.empty()) return false;
    auto& shard = shard_for(key);
    std::unique_lock lock(shard.mutex);
    Entry entry{value, std::nullopt};
    if (ttl && ttl->count() > 0) {
        entry.expires_at = std::chrono::steady_clock::now() + *ttl;
    }
    shard.data[key] = std::move(entry);
    return true;
}

std::optional<std::string> KeyValueStore::get(const std::string& key) {
    auto& shard = shard_for(key);
    {
        std::shared_lock lock(shard.mutex);
        auto it = shard.data.find(key);
        if (it == shard.data.end()) return std::nullopt;
        if (!is_expired(it->second)) return it->second.value;
    }
    // Lazy expiration under exclusive lock.
    std::unique_lock lock(shard.mutex);
    auto it = shard.data.find(key);
    if (it != shard.data.end() && is_expired(it->second)) shard.data.erase(it);
    return std::nullopt;
}

bool KeyValueStore::exists(const std::string& key) {
    return get(key).has_value();
}

bool KeyValueStore::remove(const std::string& key) {
    auto& shard = shard_for(key);
    std::unique_lock lock(shard.mutex);
    return shard.data.erase(key) > 0;
}

std::size_t KeyValueStore::size() {
    std::size_t total = 0;
    for (auto& shard : shards_) {
        std::unique_lock lock(shard.mutex);
        for (auto it = shard.data.begin(); it != shard.data.end();) {
            if (is_expired(it->second)) it = shard.data.erase(it);
            else { ++total; ++it; }
        }
    }
    return total;
}

void KeyValueStore::clear() {
    for (auto& shard : shards_) {
        std::unique_lock lock(shard.mutex);
        shard.data.clear();
    }
}

std::vector<std::pair<std::string, std::string>> KeyValueStore::snapshot() {
    std::vector<std::pair<std::string, std::string>> result;
    for (auto& shard : shards_) {
        std::unique_lock lock(shard.mutex);
        for (auto it = shard.data.begin(); it != shard.data.end();) {
            if (is_expired(it->second)) {
                it = shard.data.erase(it);
            } else {
                result.emplace_back(it->first, it->second.value);
                ++it;
            }
        }
    }
    return result;
}

} // namespace kv
