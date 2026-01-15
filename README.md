# mt-logparser

Multithreaded log parser for nginx/apache access logs written in C.

## Features

- Multithreaded parsing using POSIX threads
- Memory-mapped file I/O for optimal performance
- Custom hash table implementation for IP tracking
- Heap sort for top IP ranking
- Detailed statistics on HTTP methods, status codes, bandwidth, and IP addresses
- ~4x speedup with 4 threads on typical workloads

## Build

```bash
make
```

Requirements:
- GCC or compatible C compiler
- POSIX threads library
- Make

## Usage

Place your access log file as `access.log` in the project directory, then run:

```bash
./parser
```

The parser will output:
- Total requests and bandwidth
- Top 10 IP addresses by request count
- HTTP status code distribution
- HTTP method distribution
- Average bandwidth per request
- Performance comparison (single-threaded vs multithreaded)

## Performance

Benchmark on 300,000 log entries:
- Without threads: ~19ms
- With 4 threads: ~5ms
- Speedup: 3.85x (96% efficiency)

## Project Structure

```
src/
├── main.c          Thread management and orchestration
├── parser.c        Log parsing logic
├── stats.c         Statistics calculation and display
├── hash_table.c    Custom hash table implementation
└── sort.c          Heap sort implementation

include/
├── parser.h
├── stats.h
├── hash_table.h
└── sort.h
```

## Technical Details

- Uses mmap for zero-copy file reading
- Each thread parses a separate chunk of the file
- Local statistics per thread to avoid mutex contention
- Results merged after completion
- Memory-safe with proper error handling
