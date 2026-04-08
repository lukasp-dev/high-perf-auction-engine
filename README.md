# 🎨 High-Performance Digital Art Auction Engine (C++20)

An ultra-low latency, high-throughput auction matching engine designed for real-time digital art bidding. Engineered with **Mechanical Sympathy** to exploit modern ARM64 architectures, specifically optimized for Apple Silicon (M4 Pro).

## 🚀 Performance Benchmarks (Apple M4 Pro)
The engine has shattered the initial target of 100K TPS, achieving over **24 Million Transactions Per Second (TPS)** on a single thread.

| Metric | Result (1M Bids) | Result (10M Bids) |
| :--- | :--- | :--- |
| **Throughput** | **28.37M TPS** | **24.10M TPS** |
| **Avg Latency** | **35.24 ns/bid** | **41.48 ns/bid** |
| **Determinism** | Ultra-Stable P99 | L1/L2 Cache Optimized |

> **Note:** These results represent the **Full Pipeline** performance, including Zero-copy Parsing, Price-Time Priority Matching, and Trade Tape Logging.

---

## 🛠️ Architectural Core Values

### 1. Zero-Copy & Zero-Allocation Path
- **Non-owning Strings**: Utilized `std::string_view` for parsing raw CSV buffers, eliminating heap allocations (`malloc`/`free`) during the hot path.
- **Fast Numeric Parsing**: Implemented custom ARM64-optimized `fast_atof` and `std::from_chars` for locale-independent, high-speed numeric conversion.

### 2. Mechanical Sympathy (Hardware Optimization)
- **Cache-Line Alignment**: All critical data structures (`Bid`, `TradeEvent`) are `alignas(64)` to prevent **False Sharing** and ensure perfect L1/L2 cache locality.
- **ARM64 Native Optimization**: Compiled with `-mcpu=native` and `-ffast-math` to leverage M4 Pro’s advanced execution units and SIMD capabilities.

### 3. High-Fidelity Trade Tape
- **Event Streaming**: Instead of just tracking the final winner, the engine maintains a **Trade Tape** (Time & Sales) that records every price discovery event with nanosecond precision.
- **Pre-allocated Buffers**: Used Arena-style memory pre-allocation for the trade tape to eliminate latency spikes during volatile market conditions.

---

## 📍 Project Roadmap

- [x] **Phase 1: Zero-copy Parsing** — Achieving < 50ns parsing latency.
- [x] **Phase 2: Matching Logic** — Implementing Price-Time Priority & Order Books.
- [ ] **Phase 3: Concurrency & Sharding** — Multi-threaded Dispatching for 100M+ TPS.
- [ ] **Phase 4: Real-time I/O** — Integration with `kqueue` (macOS) / `epoll` (Linux) for TCP/UDP bidding.

---

## 💻 Technical Stack
- **Language:** C++20 (Standard: `-std=c++20`)
- **Compiler:** Apple Clang (LLVM)
- **Build System:** CMake (Optimized for Apple Silicon)
- **Target Hardware:** ARM64 (Apple M-Series / AWS Graviton)
