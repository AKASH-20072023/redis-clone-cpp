# Redis-like In-Memory Key-Value Store (C++)

A simplified Redis-like in-memory key-value store built from scratch in C++ to understand the core concepts behind key-value databases, TCP client-server communication, TTL-based expiration, and concurrent request handling.

## Features

- TCP client-server communication using Linux socket programming
- In-memory key-value storage using `std::unordered_map`
- `SET` command for storing key-value pairs
- `GET` command for retrieving values
- `DEL` command for deleting one or multiple keys
- Support for multi-word values
- TTL (Time-To-Live) using the `EX` option
- Automatic removal of expired keys when accessed
- Multiple clients handled concurrently using `std::thread`
- Thread-safe database access using `std::mutex`
- Client connection and disconnection handling

## Tech Stack

- C++
- STL
- Linux Socket Programming
- TCP/IP
- Multithreading
- Mutex / Synchronization
- Hash Tables
- C++ Chrono

## Architecture

```text
          Client 1
              │
          Client 2
              │
          Client 3
              │
              ▼
      ┌───────────────┐
      │   TCP Server  │
      │   accept()    │
      └───────┬───────┘
              │
      Creates a thread
      for each client
              │
    ┌─────────┼─────────┐
    ▼         ▼         ▼
 Thread 1  Thread 2  Thread 3
    │         │         │
    └─────────┼─────────┘
              ▼
    ┌───────────────────┐
    │  Shared Database  │
    │  unordered_map    │
    └─────────┬─────────┘
              │
           db_mutex
```


## Project Structure

```text
redis-clone-cpp/
│
├── socket_main.cpp
├── .gitignore
└── README.md
```

## How to Run

-> COMPILE 
g++ socket_main.cpp -o socket_main -pthread

-> Start the Server
127.0.0.1:6379

-> Connect as a Client(Using netcat)
nc 127.0.0.1 6379

## Example commands:

SET name Akash Chatterjee
GET name
DEL name

## TTL Example 

SET city Kolkata EX 10
GET city
-> After 10 Seconds 
GET city
(nil)

## Learning Goals

This project was built to understand how an in-memory key-value database can be implemented internally rather than simply using an existing database.

## The project combines:

Data structures
Socket programming
Client-server architecture
Operating-system concepts
Multithreading
Synchronization
TTL-based key expiration
Future Improvements

## Possible future improvements include:

Thread pool instead of creating a thread per client
Better command parsing and validation
More Redis-like commands
LRU-based memory eviction
Persistent storage
Improved TCP request/response handling
Performance benchmarking
