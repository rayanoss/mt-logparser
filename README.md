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

## Example Output

```
Threaded Parsing Results
========================
Total requests: 300000
Total bandwidth: 1517582780 bytes

─────────────────────────────────────
TOP 10 IP ADDRESSES
─────────────────────────────────────
 1. 192.168.1.14       6271 requests (2.1%)
 2. 192.168.1.46       6259 requests (2.1%)
 3. 192.168.1.38       6259 requests (2.1%)
 4. 192.168.1.28       6240 requests (2.1%)
 5. 192.168.1.17       6238 requests (2.1%)
 6. 192.168.1.4        6238 requests (2.1%)
 7. 192.168.1.43       6224 requests (2.1%)
 8. 192.168.1.36       6204 requests (2.1%)
 9. 192.168.1.35       6198 requests (2.1%)
10. 192.168.1.26       6190 requests (2.1%)
─────────────────────────────────────
STATUS CODES
─────────────────────────────────────
200 OK                         149706 (49.9%)
401 Unauthorized                49963 (16.7%)
404 Not Found                   50229 (16.7%)
500 Internal Server Error       50102 (16.7%)
─────────────────────────────────────
HTTP METHODS
─────────────────────────────────────
GET          74699 (24.9%)
POST         74722 (24.9%)
PUT          75683 (25.2%)
DELETE       74896 (25.0%)
─────────────────────────────────────
BANDWIDTH
─────────────────────────────────────
Total transferred:            1517.6 MB
Average per request:             5.1 KB

Performance Statistics
======================
Without threads: 32.114 ms
With threads (4): 8.812 ms
Speedup: 3.64x
```

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
