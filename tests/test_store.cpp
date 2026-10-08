#include "kv/key_value_store.h"
#include "kv/protocol.h"
#include "kv/persistence.h"
#include "kv/thread_pool.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

void test_basic_operations() {
    kv::KeyValueStore store(4);
    assert(store.set("name", "Girisha"));
    assert(store.get("name") == "Girisha");
    assert(store.exists("name"));
    assert(store.remove("name"));
    assert(!store.exists("name"));
}

void test_ttl() {
    kv::KeyValueStore store(4);
    assert(store.set("temp", "123", 1s));
    assert(store.get("temp") == "123");
    std::this_thread::sleep_for(1200ms);
    assert(!store.get("temp"));
}

void test_concurrency() {
    kv::KeyValueStore store(16);
    constexpr int thread_count = 8;
    constexpr int operations = 5000;
    std::vector<std::thread> threads;
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < operations; ++i) {
                const std::string key = "key_" + std::to_string((t * operations + i) % 100);
                store.set(key, std::to_string(i));
                store.get(key);
            }
        });
    }
    for (auto& t : threads) t.join();
    assert(store.size() <= 100);
    assert(store.size() > 0);
}

void test_thread_pool() {
    kv::ThreadPool pool(4);
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool.submit([i] { return i * i; }));
    }
    long long sum = 0;
    for (auto& f : futures) sum += f.get();
    assert(sum > 0);
    pool.shutdown();
}

void test_persistence() {
    const auto path = std::filesystem::temp_directory_path() / "kv_store_test.snapshot";
    std::filesystem::remove(path);
    kv::KeyValueStore store(4);
    kv::Persistence persistence(path);
    store.set("a", "one");
    store.set("b", "two");
    assert(persistence.save(store));

    kv::KeyValueStore restored(4);
    assert(persistence.load(restored));
    assert(restored.get("a") == "one");
    assert(restored.get("b") == "two");
    std::filesystem::remove(path);
}

void test_protocol() {
    kv::KeyValueStore store;
    kv::Persistence persistence(std::filesystem::temp_directory_path() / "protocol.snapshot");
    assert(kv::protocol::process_command("SET city Pune", store, persistence) == "OK");
    assert(kv::protocol::process_command("GET city", store, persistence) == "VALUE Pune");
    assert(kv::protocol::process_command("EXISTS city", store, persistence) == "1");
    assert(kv::protocol::process_command("DELETE city", store, persistence) == "OK");
    assert(kv::protocol::process_command("GET city", store, persistence) == "NOT_FOUND");
}

int main() {
    test_basic_operations();
    test_ttl();
    test_concurrency();
    test_thread_pool();
    test_persistence();
    test_protocol();
    std::cout << "All tests passed.\n";
}
