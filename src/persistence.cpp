#include "kv/persistence.h"
#include "kv/key_value_store.h"

#include <fstream>

namespace kv {

Persistence::Persistence(std::filesystem::path file) : file_(std::move(file)) {}

bool Persistence::save(const KeyValueStore& store) {
    std::lock_guard lock(mutex_);
    try {
        if (file_.has_parent_path()) std::filesystem::create_directories(file_.parent_path());
        const auto tmp = file_.string() + ".tmp";
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) return false;
        // Length-prefixed text format: key length, value length, key, value.
        for (const auto& [key, value] : const_cast<KeyValueStore&>(store).snapshot()) {
            out << key.size() << ' ' << value.size() << '\n' << key << value << '\n';
        }
        out.close();
        if (!out) return false;
        std::error_code ec;
        std::filesystem::rename(tmp, file_, ec);
        if (ec) {
            std::filesystem::remove(file_, ec);
            ec.clear();
            std::filesystem::rename(tmp, file_, ec);
        }
        return !ec;
    } catch (...) {
        return false;
    }
}

bool Persistence::load(KeyValueStore& store) {
    std::lock_guard lock(mutex_);
    std::ifstream in(file_);
    if (!in) return false;
    store.clear();
    std::size_t key_len{}, value_len{};
    while (in >> key_len >> value_len) {
        in.get();
        std::string key(key_len, '\0');
        std::string value(value_len, '\0');
        in.read(key.data(), static_cast<std::streamsize>(key_len));
        in.get();
        in.read(value.data(), static_cast<std::streamsize>(value_len));
        in.get();
        if (!in) return false;
        store.set(key, value);
    }
    return true;
}

} // namespace kv
