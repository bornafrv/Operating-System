# 🎮 Multiplayer Rock–Paper–Scissors – TCP Socket Programming in C++

This project implements a **real-time multiplayer Rock–Paper–Scissors game** using low-level **TCP socket programming** in C++. It features a central server that manages multiple game rooms and handles player interactions over concurrent connections.

---

## 🎯 Project Goals

- Master **TCP/IP socket programming** using C++.
- Implement a **scalable multiplayer architecture** with real-time responsiveness.
- Practice **multi-process and event-driven** networking using `fork`, `select`, and `poll`.
- Handle **connection management**, game state, and player interactions efficiently.

---

## 🕹️ Gameplay Overview

- Each room hosts **exactly two players**.
- Players submit moves (`rock`, `paper`, `scissors`) within **10 seconds**.
- If a player fails to respond in time, they **automatically lose**.
- Results are **broadcasted** to both players after each round.
- Players can **rejoin** or **exit** with graceful handling.

---

### 🔹 Server

- Accepts incoming connections and assigns players to rooms.
- Controls game flow, tracks room state, enforces timeouts.
- Manages concurrent games using `select()` for event-driven I/O.

### 🔹 Client

- Connects to the server and interacts via CLI.
- Submits user input, receives game state, and displays results.
- Supports basic commands such as `/end_game`, `/exit`, and `/replay`.

---

## ✨ Features

- ✅ Multiple Room Support (configurable at runtime)
- ✅ Turn-based logic with strict **10-second timeout**
- ✅ Broadcast results using efficient `write()` calls
- ✅ Handles abrupt disconnection or client failure
- ✅ Interactive CLI for user experience
- ✅ Graceful shutdown and room reset with `/end_game`

---

## 🚀 Run Instructions
🔹 Server Side

./server.out {IP_ADDRESS} {PORT} {NUM_ROOMS}

Example:

./server.out 127.0.0.1 8080 5

🔹 Client Side

./client.out {IP_ADDRESS} {PORT}

Example:


./client.out 127.0.0.1 8080

---

## 📡 Technologies Used

C++ system sockets: <sys/socket.h>, <arpa/inet.h>, <netinet/in.h>

fork(), select(), and poll() for concurrent handling

File descriptors and low-level read()/write() I/O

pipe, exec, and manual string parsing for command handling

