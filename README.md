# cutils

lightweight c utility library designed for embedded systems. it provides essential data structures and utilities with a focus on memory efficiency and safety.

## features

- memory-efficient data structures:
  - vector (dynamic array)
  - list (linked list)
  - map (key-value store)
  - queue and priority queue
  - stack
  - string utilities

- memory management:
  - static allocation support (64KB limit by default)
  - arena allocator
  - custom allocator interface

- safety features:
  - bounds checking
  - overflow protection
  - null pointer checks
  - assertions

- real-time support:
  - bounded operation times
  - priority queue with custom comparison
  - no dynamic memory by default

## building

```bash
mkdir build && cd build
cmake ..
make
```
