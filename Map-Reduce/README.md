# 🗃️ Distributed Inventory Analytics using Map-Reduce in C++

This project simulates a **distributed inventory and profit tracking system** using a **Map-Reduce architecture** in C++. Leveraging multi-process programming with `fork`, `exec`, and `pipe`, it analyzes warehouse transaction logs in parallel and generates consolidated inventory and profit reports.

---

## 🎯 Objectives

- Master **inter-process communication (IPC)** using unnamed pipes.
- Implement the **Map-Reduce paradigm** in a Unix-based environment.
- Practice system-level operations such as `fork`, `exec`, and stream redirection.
- Simulate real-world inventory tracking across distributed warehouses.

---

## 🧾 Problem Description

A food distribution company maintains inventory across several warehouses in different cities. Each warehouse logs daily transactions in a separate CSV file in the following format:

<item_name>,<quantity>,<unit_price>,<type>


Where `<type>` is either `input` (restock) or `output` (sale). The goal is to:

- Monitor real-time stock levels per item and warehouse.
- Compute **profit** from sold items (`output` transactions).
- Generate:
  - Individual reports per warehouse
  - A global master report aggregating all warehouses

---

## ⚙️ System Architecture

### 🗺️ Map Phase
- Each warehouse CSV is processed by a **dedicated child process**.
- Child reads, parses, and summarizes its local transactions.
- Sends intermediate results (item quantities and profits) to the parent via **pipes**.

### 🔁 Reduce Phase
- Parent collects results from all child processes.
- Aggregates quantities and profits per item.
- Outputs final per-item inventory and profit report.

---

## 🔌 Inter-Process Communication (IPC)

- Communication is done through **unnamed pipes** (`pipe()`).
- Data is structured and serialized for transmission.
- Pipes maintain **FIFO** order to preserve message integrity.
- Global item list is read from `parts.csv`.

---

## 💻 Features

- ✅ Supports **unlimited** number of warehouse files (scalable).
- ✅ Handles **reorders**, duplicate entries, and invalid formats gracefully.
- ✅ Clear **CLI interface** for selecting files and viewing outputs.
- ✅ Modular code structure for `map`, `reduce`, and `report` components.
- ✅ All logs are redirected for **debugging and verification**.

---

## 🧾 Output Reports

- **Warehouse Report**: Individual stock and profit per warehouse.
- **Master Report**: Consolidated inventory and total profit across all warehouses.

---

## 🏫 Course Context

> **Operating Systems – Fall 2024**  
> University of Tehran – Dept. of Electrical & Computer Engineering  
> Instructor: Prof. [Your Professor's Name]

---

## 🛠️ Compile & Run

```bash
g++ map_reduce.cpp -o inventory_system
./inventory_system warehouse1.csv warehouse2.csv ...

📌 Notes

Make sure your CSV files follow the correct format.

parts.csv must exist to validate item names.

Run on Unix-like systems (Linux/macOS) with standard C++ compiler.

