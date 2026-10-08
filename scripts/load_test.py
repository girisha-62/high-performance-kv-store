#!/usr/bin/env python3
import argparse
import socket
import threading
import time

parser = argparse.ArgumentParser(description="Simple concurrent load test for KV server")
parser.add_argument("--host", default="127.0.0.1")
parser.add_argument("--port", type=int, default=6379)
parser.add_argument("--clients", type=int, default=20)
parser.add_argument("--operations", type=int, default=1000)
args = parser.parse_args()

success = 0
failure = 0
lock = threading.Lock()

def worker(client_id):
    global success, failure
    try:
        with socket.create_connection((args.host, args.port), timeout=5) as s:
            f = s.makefile("rwb")
            for i in range(args.operations):
                key = f"client{client_id}_key{i % 100}".encode()
                f.write(b"SET " + key + b" value\n")
                f.flush()
                if f.readline().strip() != b"OK":
                    raise RuntimeError("SET failed")
                f.write(b"GET " + key + b"\n")
                f.flush()
                if not f.readline().startswith(b"VALUE "):
                    raise RuntimeError("GET failed")
        with lock:
            success += args.operations * 2
    except Exception:
        with lock:
            failure += 1

start = time.perf_counter()
threads = [threading.Thread(target=worker, args=(i,)) for i in range(args.clients)]
for t in threads: t.start()
for t in threads: t.join()
elapsed = time.perf_counter() - start

total = args.clients * args.operations * 2
print(f"Clients: {args.clients}")
print(f"Operations per client: {args.operations}")
print(f"Total operations: {total}")
print(f"Successful operations: {success}")
print(f"Failed clients: {failure}")
print(f"Elapsed: {elapsed:.3f} s")
print(f"Throughput: {success / elapsed:.2f} ops/sec" if elapsed else "Throughput: n/a")
