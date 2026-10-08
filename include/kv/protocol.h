#pragma once

#include <string>
#include <vector>

namespace kv {
class KeyValueStore;
class Persistence;

namespace protocol {
std::vector<std::string> split_command(const std::string& line);
std::string process_command(const std::string& line, KeyValueStore& store,
                            Persistence& persistence);
} // namespace protocol
} // namespace kv
