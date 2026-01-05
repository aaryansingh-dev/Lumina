# Lumina

Lumina is a high-performance key-value storage engine written in modern C++, inspired by LevelDB and BigTable. It implements an LSM-tree architecture with a strong focus on durability, crash consistency, and efficient disk I/O.

## Features

- Write-Ahead Log (WAL) for full durability
- In-memory MemTable with ordered indexing
- Immutable on-disk SSTables
- Crash-safe recovery via WAL replay
- Background compaction
- Bloom filters to reduce read amplification

## Project Structure

```text
lumina/
├── include/        # Public API headers
├── src/            # Internal implementation
├── tests/          # Unit & recovery tests
├── benchmarks/     # Performance benchmarks
└── CMakeLists.txt  # Build configuration
```

## Goals
Goals

Lumina is designed as a systems-focused project to explore:

- disk-backed data structures

- write amplification and compaction

- crash consistency and recovery

- low-latency storage design


## Status 

Under Development

## License

MIT License

Copyright (c) 2026 Aaryan Singh

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
