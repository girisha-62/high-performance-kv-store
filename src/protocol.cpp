#include "kv/protocol.h"
#include "kv/key_value_store.h"
#include "kv/persistence.h"

#include <sstream>
#include <stdexcept>

namespace kv::protocol {

std::vector<std::string> split_command(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

std::string process_command(const std::string& line, KeyValueStore& store,
                            Persistence& persistence) {
    auto t = split_command(line);
    if (t.empty()) return "ERR empty command";
    const std::string& cmd = t[0];

    if (cmd == "SET") {
        if (t.size() != 3 && t.size() != 4) return "ERR usage: SET key value [ttl_seconds]";
        std::optional<std::chrono::seconds> ttl;
        if (t.size() == 4) {
            try { ttl = std::chrono::seconds(std::stoll(t[3])); }
            catch (...) { return "ERR invalid ttl"; }
            if (ttl->count() <= 0) return "ERR ttl must be positive";
        }
        return store.set(t[1], t[2], ttl) ? "OK" : "ERR invalid key";
    }
    if (cmd == "GET") {
        if (t.size() != 2) return "ERR usage: GET key";
        auto value = store.get(t[1]);
        return value ? "VALUE " + *value : "NOT_FOUND";
    }
    if (cmd == "DELETE") {
        if (t.size() != 2) return "ERR usage: DELETE key";
        return store.remove(t[1]) ? "OK" : "NOT_FOUND";
    }
    if (cmd == "EXISTS") {
        if (t.size() != 2) return "ERR usage: EXISTS key";
        return store.exists(t[1]) ? "1" : "0";
    }
    if (cmd == "SIZE") {
        if (t.size() != 1) return "ERR usage: SIZE";
        return "SIZE " + std::to_string(store.size());
    }
    if (cmd == "SAVE") {
        if (t.size() != 1) return "ERR usage: SAVE";
        return persistence.save(store) ? "OK" : "ERR save failed";
    }
    if (cmd == "LOAD") {
        if (t.size() != 1) return "ERR usage: LOAD";
        return persistence.load(store) ? "OK" : "ERR load failed";
    }
    if (cmd == "CLEAR") {
        if (t.size() != 1) return "ERR usage: CLEAR";
        store.clear();
        return "OK";
    }
    if (cmd == "PING") {
        return t.size() == 1 ? "PONG" : "ERR usage: PING";
    }
    if (cmd == "QUIT") {
        return t.size() == 1 ? "BYE" : "ERR usage: QUIT";
    }
    return "ERR unknown command";
}

} // namespace kv::protocol
