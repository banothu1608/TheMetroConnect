# 🚇 Hyderabad Metro Connect

A console-based **C++ application** that simulates the Hyderabad Metro Rail network, allowing users to find the **shortest route**, **minimum distance**, and **minimum travel time** between any two stations across all three metro lines — using **Graph** and **Dijkstra's Algorithm**.

---

## 📌 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Data Structures & Algorithm](#data-structures--algorithm)
- [Metro Network](#metro-network)
- [How It Works](#how-it-works)
- [Algorithm Steps](#algorithm-steps)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Sample Output](#sample-output)
- [Key Design Decisions](#key-design-decisions)
- [Tech Stack](#tech-stack)
- [Future Improvements](#future-improvements)

---

## Overview

Hyderabad Metro Connect models the real Hyderabad Metro Rail network as a **weighted undirected graph**, where:

- **Nodes** represent metro stations, each tagged with its line corridor (`^R` Red, `^B` Blue, `^G` Green)
- **Edges** represent direct connections between adjacent stations, weighted by **distance in kilometres**
- **Interchange edges** connect the same physical station across different lines (Ameerpet, MG Bus Station, Paradise), modelling real-world platform transfers

The program uses **Dijkstra's algorithm** to compute both the shortest distance path and the fastest time path between any source and destination station.

---

## Features

| Feature | Description |
|---|---|
| 🗺️ Full Metro Map | Displays all 59 stations with their neighbours and distances |
| 📋 Station List | Alphabetically sorted list of all stations with line badges |
| 📏 Shortest Distance | Minimum kilometre path between any two stations |
| ⏱️ Shortest Time | Fastest route accounting for travel speed and stop time |
| 🛤️ Full Route (Distance) | Step-by-step path optimised for distance |
| 🛤️ Full Route (Time) | Step-by-step path optimised for travel time |
| 🔍 Fuzzy Station Search | Type `Miyapur` instead of `Miyapur~R` — auto-resolved |
| 🔀 Smart Disambiguation | Multiple line matches prompt a numbered selection menu |
| ♻️ Cross-platform Clear | Works on Windows, Linux, and macOS terminals |

---

## Data Structures & Algorithm

### Graph (Adjacency List)

```
unordered_map<string, Vertex>   →   each station maps to its neighbours
unordered_map<string, int>      →   each neighbour maps to edge weight (km)
```

The graph is **undirected** — every `addEdge(A, B, w)` call inserts both `A→B` and `B→A`.

### Min-Heap (Priority Queue)

```cpp
priority_queue<DijkstraPair>   // min-heap via reversed comparator
```

Used inside Dijkstra to always expand the lowest-cost unvisited node next.

### Dijkstra's Algorithm

Two modes are supported via a single boolean flag:

| Mode | Edge weight formula |
|---|---|
| Distance-wise | `w = actual_km` |
| Time-wise | `w = 120 + 40 × actual_km` (seconds: 120s stop time + speed factor) |

Parent tracking (`unordered_map<string, string>`) enables full path reconstruction by backtracing from destination to source.

### STL Components Used

| Component | Purpose |
|---|---|
| `unordered_map` | O(1) average station lookup and adjacency list |
| `priority_queue` | Min-heap for Dijkstra's greedy expansion |
| `vector` | Path reconstruction and sorted display |
| `ostringstream` | Formatted route output string building |
| `algorithm` (sort) | Alphabetical station and neighbour ordering |

---

## Metro Network

### Lines & Stations

| Line | Colour | Route | Stations |
|---|---|---|---|
| Red Line | 🔴 `^R` | Miyapur ↔ LB Nagar | 27 stations |
| Blue Line | 🔵 `^B` | Nagole ↔ Raidurg | 22 stations |
| Green Line | 🟢 `^G` | Jubilee Bus Station ↔ MG Bus Station | 10 stations |

**Total: 59 station nodes, 3 interchange nodes**

### Interchange Stations

| Station | Lines Connected | Transfer Cost |
|---|---|---|
| Ameerpet | Red ↔ Blue | 1 km |
| MG Bus Station | Red ↔ Green | 1 km |
| Paradise | Blue ↔ Green | 1 km |

> Each interchange is modelled as **two separate nodes** (e.g. `Paradise~B` and `Paradise~G`) connected by an interchange edge — matching how real transit software handles multi-line stations.

---

## How It Works

```
User Input (plain name)
        │
        ▼
resolveStation()  ──── exact match? ────► use directly
        │
        ├── display form "Name ^X"? ──► convert to "Name~X"
        │
        └── substring match ──► 1 result? use it
                              └── multiple? show numbered menu → user picks
        │
        ▼
Dijkstra (distance or time mode)
        │
        ▼
Path reconstruction via parent map
        │
        ▼
Formatted output with line badges
```

---

## Algorithm Steps

1. **Build the graph** — all 59 station vertices and edges are registered at startup via `Create_Metro_Map()`
2. **Accept user input** — station names are resolved via fuzzy matching; ambiguous names trigger a selection prompt
3. **Initialise Dijkstra** — all distances set to `INT_MAX`; source set to `0`; pushed onto min-heap
4. **Greedy expansion** — repeatedly extract the minimum-cost node; relax all neighbours
5. **Mode switching** — distance mode uses raw km; time mode applies `120 + 40×km` per edge
6. **Path reconstruction** — backtrack from destination using the `parent` map; reverse to get source→destination order
7. **Display** — numbered station list with line badges and total cost

---

## Project Structure

```
HyderabadMetroConnect/
│
├── Main.cpp          # Complete source — graph, Dijkstra, UI, metro data
└── README.md         # This file
```

> The entire project is self-contained in a single `.cpp` file with no external dependencies.

---

## Getting Started

### Prerequisites

- A C++17 compatible compiler (`g++`, `clang++`, MSVC)
- Any IDE or terminal: VS Code, Code::Blocks, Visual Studio, CLion, or plain terminal

### Compile & Run

**Linux / macOS**
```bash
g++ -std=c++17 -O2 -o metro Main.cpp
./metro
```

**Windows (MinGW)**
```bash
g++ -std=c++17 -O2 -o metro.exe Main.cpp
metro.exe
```

**Windows (MSVC — Developer Command Prompt)**
```bash
cl /std:c++17 /EHsc Main.cpp /Fe:metro.exe
metro.exe
```

> No third-party libraries required. Compiles cleanly with zero warnings under `-Wall -Wextra`.

---

## Sample Output

### Shortest path — Miyapur → Raidurg

```
Enter source station      : Miyapur
Enter destination station : Raidurg

  Route (Distance Wise): Miyapur ^R → Raidurg ^B
   1. Miyapur ^R
   2. JNTU College ^R
   3. KPHB Colony ^R
   4. Kukatpally ^R
   5. Balanagar ^R
   6. Moosapet ^R
   7. Bharat Nagar ^R
   8. Erragadda ^R
   9. ESI Hospital ^R
  10. SR Nagar ^R
  11. Ameerpet ^R
  12. Ameerpet ^B       ← line interchange
  13. Madhura Nagar ^B
  14. Yousufguda ^B
  15. Road No 5 Jubilee Hills ^B
  16. Jubilee Hills Check Post ^B
  17. Peddamma Temple ^B
  18. Madhapur ^B
  19. Durgam Cheruvu ^B
  20. Hi-Tech City ^B
  21. Raidurg ^B
  Total Distance: 31 KM
```

### Disambiguation — Ameerpet (exists on two lines)

```
Enter source station      : Ameerpet
  Multiple stations match "Ameerpet":
    1. Ameerpet ^B
    2. Ameerpet ^R
  Enter number (or 0 to re-type): 2
```

### Unknown station — graceful re-prompt

```
Enter source station      : Koti
  ✗ No station found matching "Koti". Please try again.
Enter source station      :
```

---

## Key Design Decisions

### Why two nodes per interchange station?
Using a single merged node (e.g. the original `Paradise~BG`) loses the ability to represent the real-world walking transfer cost between platforms. Splitting into `Paradise~B` and `Paradise~G` with a weighted interchange edge reflects how professional transit software (GTFS, OpenTripPlanner) models multi-modal transfers.

### Why fuzzy name resolution?
Requiring users to type `Miyapur~R` is an implementation detail leaking into the UI. The internal `~X` suffix is a storage key — users should never see or type it. The `resolveStation()` function bridges this gap cleanly.

### Why no `system("clear")`?
`system("clear")` is Linux-only and spawns a subshell — a code smell flagged in professional reviews. The custom `clearScreen()` function uses ANSI escape codes on POSIX and the WinAPI on Windows, keeping the binary dependency-free.

---

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C++17 |
| Core DSA | Graph (adjacency list), Min-Heap, Dijkstra's Algorithm |
| STL | `unordered_map`, `priority_queue`, `vector`, `algorithm`, `sstream` |
| Build | `g++` / `clang++` / MSVC — no build system required |
| Dependencies | None (standard library only) |

---

## Future Improvements

- [ ] Fare calculation based on distance slabs (like real Hyderabad Metro pricing)
- [ ] Interactive metro map rendering in terminal (ASCII art or ncurses)
- [ ] Support for upcoming metro extensions (Phase II corridors)
- [ ] Input validation for non-integer menu entries
- [ ] File-based graph loading to support network updates without recompilation
- [ ] Unit tests for Dijkstra correctness on known routes

---

## 👨‍💻 banothu1608

Built as a Data Structures & Algorithms project demonstrating practical application of **Graph theory** and **Greedy shortest-path algorithms** on a real-world transit network.

---

