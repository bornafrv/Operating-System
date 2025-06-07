# 🎧 Audio Signal Filtering with Multi-Threaded Processing in C++

This project implements a **multi-threaded audio signal processing system** in C++ to apply digital filters on `.wav` audio files. By leveraging **POSIX threads**, it enables real-time and efficient filtering, with support for both **serial** and **parallel** execution modes.

---

## 🎯 Project Goals

- Apply core **Digital Signal Processing (DSP)** filters to audio signals.
- Learn and utilize **multi-threading** using `pthreads`.
- Benchmark and compare serial vs parallel performance.
- Build a CLI-based, real-time audio filtering tool.

---

## 🔍 Supported Filters

| Filter Type     | Description                                                                 |
|------------------|-----------------------------------------------------------------------------|
| **Band-pass**     | Passes only a specified frequency range; blocks others.                    |
| **Notch**         | Suppresses a single frequency (e.g. 50 Hz electrical hum).                 |
| **FIR**           | Finite Impulse Response (feedforward, inherently stable).                  |
| **IIR**           | Infinite Impulse Response (feedback-based, more efficient but complex).    |

All filters operate directly on audio buffers and are tested for time-domain correctness.

---

## ⚙️ Features

- ✅ **WAV audio support** via `libsndfile`.
- ✅ Interactive **Command-Line Interface** for filter selection and file input.
- ✅ Efficient **multi-threaded processing** using `pthreads`.
- ✅ Benchmarking output showing **execution time and speed-up**.
- ✅ Automatically saves filtered output as `output.wav`.

---

## 🛠️ Build Instructions

Ensure `libsndfile` is installed on your system. Then run:

```bash
make

```

## 🚀 Run Instructions
🔹 Serial Mode

./VoiceFilters.out serial input.wav

🔹 Parallel Mode

./VoiceFilters.out parallel input.wav

## 📈 Performance Evaluation

The system outputs the runtime for both modes and calculates:

Speed-up = Time_serial / Time_parallel

You can test performance for different audio sizes and thread configurations to study scalability and parallel efficiency.

## 📚 Dependencies

C++11 or later

libsndfile (audio file I/O)

POSIX threads (pthread.h)