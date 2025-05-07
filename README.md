# cutils

A lightweight C utility library designed for embedded systems. It provides essential data structures and utilities with a focus on memory efficiency and safety.

## Features

- Memory-efficient data structures:
  - Vector (dynamic array)
  - List (linked list)
  - Map (key-value store)
  - Queue and Priority Queue
  - Stack
  - String utilities

- Memory management:
  - Static allocation support (64KB limit)
  - Arena allocator
  - Custom allocator interface

- Safety features:
  - Bounds checking
  - Overflow protection
  - Null pointer checks
  - Assertions

- Real-time support:
  - Bounded operation times
  - Priority queue with custom comparison
  - No dynamic memory by default

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## License

GNU General Public License v3.0 