# 🧠 Operating Systems Projects – C++ Implementations

This repository contains a collection of advanced projects implemented in **C++** as part of the *Operating Systems* course at the **University of Tehran**. Each project is designed to deepen understanding of system-level concepts such as inter-process communication (IPC), multi-threading, and socket programming.

---

## 🧩 Included Projects

### 1. 🔄 Map-Reduce – Inventory & Profit Tracking  
📂 `Map-Reduce/`  
Implements a Map-Reduce model for parallel processing of warehouse inventory logs using `fork()`, `exec()`, and unnamed `pipe()` communication.

> **Core Concepts**: Process creation, inter-process communication, aggregation  
> **Tech Stack**: C++, UNIX system calls  

➡️ [Read the full description »](./Map-Reduce/README.md)

---

### 2. 🎧 Multi-Threaded Audio Filtering  
📂 `Multi-Threading/`  
Applies real-time digital filters (Band-pass, Notch, FIR, IIR) to `.wav` audio files using POSIX threads and compares performance between serial and parallel execution.

> **Core Concepts**: `pthreads`, digital signal processing, CPU-bound parallelism  
> **Tech Stack**: C++, `libsndfile`, `pthread.h`  

➡️ [Read the full description »](./Multi-Threading/README.md)

---

### 3. 🌐 Rock–Paper–Scissors Multiplayer Game  
📂 `Socket Programming/`  
Implements a real-time multiplayer game over **TCP sockets**. The server manages multiple rooms, enforces timeouts, and uses `select()` to support concurrent players.

> **Core Concepts**: TCP socket programming, concurrency, I/O multiplexing  
> **Tech Stack**: C++, `<sys/socket.h>`, `<poll.h>`  

➡️ [Read the full description »](./Socket%20Programming/README.md)

---

## 🛠️ Requirements

- **Compiler**: `g++` (C++11 or later)
- **System**: Linux / UNIX-based OS
- **Dependencies**:
  - `pthread` (multi-threading)
  - `libsndfile` (audio)
  - Standard socket headers (`netinet/in.h`, `sys/socket.h`)

---

## 📌 Course Information
🧾 Operating Systems
🏛️ University of Tehran – Department of Electrical and Computer Engineering
👨‍🏫 Instructor: Dr. Kargahi

