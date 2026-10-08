- High-Performance Multithreaded Key-Value Store

  A high-performance, thread-safe in-memory key-value store implemented in **C++17** with a concurrent TCP client-server architecture, sharded storage, worker threads, persistence, CMake-based builds, automated tests, and Docker support.

  The project is designed to demonstrate practical **systems programming, multithreading, networking, concurrency, synchronization, memory management, and performance-oriented C++ development**.
  ---
  ## Features
  - Thread-safe in-memory key-value storage
  - TCP client-server architecture
  - Concurrent client handling using worker threads
  - Sharded storage for improved concurrent access
  - Thread-safe operations using synchronization primitives
  - Support for multiple key-value operations
  - Data persistence with SAVE and LOAD operations
  - Efficient hash-based key lookup
  - Configurable server port and worker threads
  - CMake-based build system
  - Automated unit tests
  - Docker support
  - Linux-compatible development environment
  ---
  ## Supported Commands
  The server supports the following commands:
  | Command         | Description                               |
  | --------------- | ----------------------------------------- |
  | `SET key value` | Stores a value for a key                  |
  | `GET key`       | Retrieves the value associated with a key |
  | `DELETE key`    | Removes a key-value pair                  |
  | `EXISTS key`    | Checks whether a key exists               |
  | `SIZE`          | Returns the number of stored keys         |
  | `SAVE`          | Saves the current store to disk           |
  | `LOAD`          | Loads stored data from disk               |
  | `CLEAR`         | Removes all key-value pairs               |
  | `PING`          | Tests server connectivity                 |
  | `QUIT`          | Closes the client connection              |
  ---
  ## Architecture
  The system follows a client-server architecture:
  ```text
                     +-------------------+
                     |    TCP Client     |
                     +---------+---------+
                               |
                               | TCP
                               |
                     +---------v---------+
                     |    TCP Server     |
                     +---------+---------+
                               |
                      +--------v--------+
                      | Worker Threads  |
                      +--------+--------+
                               |
                      +--------v--------+
                      | Request Handler |
                      +--------+--------+
                               |
                      +--------v--------+
                      | Sharded Storage |
                      +--------+--------+
                               |
                      +--------v--------+
                      | Persistence     |
                      +-----------------+
  ```
  ### Request Flow
  ```text
  Client
     |
     | TCP Request
     v
  Server
     |
     v
  Worker Thread
     |
     v
  Command Parser
     |
     v
  Key-Value Store
     |
     +----> Hash-based lookup
     |
     +----> Shard selection
     |
     +----> Thread synchronization
     |
     v
  Response
     |
     v
  Client
  ```
  ---
  ## Concurrency Design
  The key-value store is designed to support concurrent client requests.

  Instead of using a single global lock for the entire database, the storage layer is divided into multiple shards.
  ```text
                      Key-Value Store
                            |
          +-----------------+-----------------+
          |                 |                 |
        Shard 0           Shard 1          Shard N
          |                 |                 |
       Lock 0            Lock 1            Lock N
  ```
  A key is mapped to a shard using its hash value.

  This allows operations on different shards to proceed concurrently while still maintaining thread safety.
  ### Benefits
  - Reduced lock contention
  - Better concurrency
  - Independent synchronization for different shards
  - Improved scalability compared with a single global lock
  ---
  ## Worker Thread Model
  The server uses a configurable number of worker threads to process client requests.

  Example:
  ```bash
  ./kv_server 6379 4
  ```
  Where:
  - `6379` = server port
  - `4` = number of worker threads
  Conceptually:
  ```text
                TCP Server
                    |
          +---------+---------+
          |         |         |
        Worker    Worker    Worker
          1         2         3
                    |
                 Worker 4
                    |
                    v
              Request Queue
                    |
                    v
             Key-Value Store
  ```
  This allows multiple client requests to be processed concurrently.
  ---
  ## Tech Stack
  ### Programming
  - C++17
  - Standard Template Library (STL)
  ### Systems & Networking
  - TCP/IP sockets
  - Multithreading
  - Thread synchronization
  - Concurrent data structures
  - Linux system programming concepts
  ### Build & Testing
  - CMake
  - CTest
  - Automated unit tests
  ### Development & Deployment
  - Linux
  - Docker
  - Docker Compose
  - Git
  - GitHub
  ---
  ## Project Structure
  ```text
  high-performance-kv-store/
  │
  ├── CMakeLists.txt
  ├── README.md
  ├── .gitignore
  ├── Dockerfile
  ├── docker-compose.yml
  │
  ├── include/
  │   └── Header files
  │
  ├── src/
  │   └── Source files
  │
  ├── tests/
  │   └── Unit tests
  │
  └── docs/
      └── Additional documentation
  ```
  ---
  # Build and Run
  ## Prerequisites
  Make sure the following tools are installed:
  - C++17 compatible compiler
  - CMake 3.15+
  - Git
  - Docker (optional)
  For Linux:
  ```bash
  sudo apt update
  sudo apt install build-essential cmake
  ```
  ---
  ## Build from Source
  Clone the repository:
  ```bash
  git clone https://github.com/<your-username>/high-performance-kv-store.git
  ```
  Move into the project directory:
  ```bash
  cd high-performance-kv-store
  ```
  Create a build directory:
  ```bash
  mkdir build
  cd build
  ```
  Configure the project:
  ```bash
  cmake ..
  ```
  Build the project:
  ```bash
  cmake --build .
  ```
  ---
  ## Run Tests
  After building:
  ```bash
  ctest --output-on-failure
  ```
  Expected result:
  ```text
  100% tests passed, 0 tests failed
  ```
  ---
  # Running the Server
  From the build directory:
  ```bash
  ./kv_server 6379 4
  ```
  Example output:
  ```text
  ========================================
        HIGH-PERFORMANCE KEY-VALUE STORE
  ========================================

  Port: 6379
  Worker threads: 4
  Shards: 16

  Commands:
  SET GET DELETE EXISTS SIZE SAVE LOAD CLEAR PING QUIT

  Waiting for clients...
  ```
  The server listens for TCP client connections on port `6379`.
  ---
  # Running the Client
  Open another terminal and run:
  ```bash
  ./kv_client 127.0.0.1 6379
  ```
  The client connects to the server using:
  ```text
  IP Address: 127.0.0.1
  Port: 6379
  ```
  ---
  # Example Usage
  ### Store a value
  ```text
  SET name Girisha
  ```
  Response:
  ```text
  OK
  ```
  ### Retrieve a value
  ```text
  GET name
  ```
  Response:
  ```text
  VALUE Girisha
  ```
  ### Check whether a key exists
  ```text
  EXISTS name
  ```
  ### Get the number of stored keys
  ```text
  SIZE
  ```
  ### Delete a key
  ```text
  DELETE name
  ```
  ### Test connectivity
  ```text
  PING
  ```
  ### Clear the store
  ```text
  CLEAR
  ```
  ### Save data
  ```text
  SAVE
  ```
  ### Load data
  ```text
  LOAD
  ```
  ### Exit
  ```text
  QUIT
  ```
  ---
  # Docker
  The project includes Docker support for reproducible builds and execution.

  Build the Docker image:
  ```bash
  docker build -t high-performance-kv-store .
  ```
  Run the server:
  ```bash
  docker run -p 6379:6379 high-performance-kv-store
  ```
  If Docker Compose is configured:
  ```bash
  docker compose build
  docker compose up
  ```
  The server can then be accessed through:
  ```text
  localhost:6379
  ```
  ---
  # Testing
  The project includes automated tests covering core key-value store functionality.

  Tests verify operations such as:
  - SET
  - GET
  - DELETE
  - EXISTS
  - SIZE
  - CLEAR
  - Concurrent access
  - Core storage behavior
  Run all tests using:
  ```bash
  ctest --output-on-failure
  ```
  ---
  # Key Engineering Concepts Demonstrated
  This project demonstrates practical knowledge of:
  ### C++
  - C++17
  - STL containers
  - RAII
  - Smart pointers
  - Object-oriented programming
- Resource management
- Exception handling

### Multithreading

- `std::thread`
- Thread pools
- Mutexes
- Locking
- Condition variables
- Concurrent request processing

### Networking

- TCP sockets
- Client-server communication
- Socket lifecycle management
- Request/response handling

### Data Structures

- Hash tables
- Hash-based key lookup
- Sharded storage

### Systems Programming

- Linux
- File I/O
- Networking
- Process and thread management
- Synchronization

### Software Engineering

- CMake
- Unit testing
- Docker
- Git
- GitHub

---

# Performance Considerations

The project uses several techniques to improve concurrent performance:

- Sharded key-value storage
- Multiple worker threads
- Hash-based key lookup
- Reduced lock contention
- Concurrent request processing
- In-memory data access

The architecture is designed to allow independent operations on different shards to execute concurrently.

---

# Future Improvements

Potential future enhancements include:

- Connection pooling
- Improved request parsing
- Asynchronous I/O
- More extensive benchmarking
- Performance metrics
- Configurable shard count
- Configurable maximum connections
- Authentication and authorization
- TLS support
- Replication
- Distributed deployment
- More comprehensive integration tests
- Load testing and benchmarking tools

---

# Why This Project?

This project was built to explore how a production-style in-memory data store can be designed using modern C++.

It focuses on the challenges involved in:

- Concurrent data access
- Thread synchronization
- Network communication
- Efficient data structures
- Server architecture
- Persistence
- Build automation
- Containerization

---

# Author

**Girisha Nerkar**

Software Engineer | C++ | Linux | SQL | Docker | Distributed & Transaction Systems

