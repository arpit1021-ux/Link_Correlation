# Link-Correlation Aware Routing in Multihop Wireless Networks

A progressive C++ simulation and analytical framework for **correlation-aware opportunistic routing** with **full-duplex concurrent forwarding** in wireless multihop networks. Organized into 6 phases, each building upon the previous to add more realistic capabilities — from basic ETX computation to full multicast delivery with validated simplex-vs-full-duplex tradeoffs.

---

## Table of Contents

- [Overview](#overview)
- [Core Concepts](#core-concepts)
- [Base Test Case](#base-test-case)
- [Phase Summary](#phase-summary)
- [Phase 1 — Basic ETX Calculator (2 Forwarders, 1 Destination)](#phase-1--basic-etx-calculator-2-forwarders-1-destination)
- [Phase 2 — 3-Forwarder Simplex ETX (Analytical)](#phase-2--3-forwarder-simplex-etx-analytical)
- [Phase 3 — 3-Forwarder Simplex vs Full Duplex (Analytical)](#phase-3--3-forwarder-simplex-vs-full-duplex-analytical)
- [Phase 4 — Multicast Simplex (Simulation + Analytical)](#phase-4--multicast-simplex-simulation--analytical)
- [Phase 5 — Simplex vs Full Duplex (Simulation + Analytical)](#phase-5--simplex-vs-full-duplex-simulation--analytical)
- [Phase 6 — Generalized Multi-Hop Network (WIP)](#phase-6--generalized-multi-hop-network-wip)
- [Validation Summary](#validation-summary)
- [How to Build & Run](#how-to-build--run)
- [Input File Format](#input-file-format)
- [Project Structure](#project-structure)

---

## Overview

Wireless networks suffer from unreliable links due to interference, fading, and shared medium effects. Traditional routing assumes independent link behavior, which is often inaccurate. This project addresses this limitation by:

- Modeling **correlated packet receptions** using exact joint probability distributions (not independence assumption)
- Using **Expected Transmission Count (ETX)** as the routing metric, extended to forwarder sets and multicast routing
- Introducing a **full-duplex concurrent forwarding model** where multiple forwarders transmit simultaneously, reducing delivery latency at the cost of increased radio resource usage
- Validating all analytical results against Monte Carlo simulation with **~1% agreement**

---

## Core Concepts

| Concept | Description |
|---------|-------------|
| **ETX (Expected Transmission Count)** | Average transmissions needed for one successful delivery. Computed via fixed-point iteration over all reception outcome regions. |
| **Venn Region Decomposition** | For correlated links, the reception space is partitioned into exact regions (none, only1, only2, both, etc.) using inclusion-exclusion on joint probabilities. |
| **Full Duplex (FD)** | All active forwarders transmit simultaneously in the same time slot. Reduces latency by 25.9% at 9.0% more radio cost. |
| **Simplex** | One forwarder transmits per time slot. Uses optimal relay selection (minimum cost per coverage state). |
| **Bitmask Coverage Engine** | Iterative fixed-point solver over all 2^D coverage states (D = number of destinations). Used for analytical multicast cost computation. |
| **L'Ecuyer RNG** | Phases 4–5 use the L'Ecuyer combined multiplicative congruential generator for reproducible, high-quality random numbers in Monte Carlo simulation. |
| **Absorbing Condition** | For 3-forwarder multicast with reachability 1→{4,5}, 2→{5,6}, 3→{5,6}: coverage is achieved when (f1 received) AND (f2 OR f3 received). |

---

## Base Test Case

All results below are computed for the following base test case. Every phase file accepts runtime input, so different probabilities can be tested by re-running with new values.

**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

```
Reachability:
  Node 1 → {4, 5}
  Node 2 → {5, 6}
  Node 3 → {5, 6}
```

**Hop-1 Probabilities (source → forwarders):**

| Marginal | Value | Joint | Value |
|----------|-------|-------|-------|
| p(0→1) | 0.50 | p(0→1,2) | 0.30 |
| p(0→2) | 0.40 | p(0→1,3) | 0.25 |
| p(0→3) | 0.45 | p(0→2,3) | 0.20 |
| | | p(0→1,2,3) | 0.15 |

**Hop-2 Probabilities (forwarders → destinations):**

| Forwarder | Marginal | Value | Joint | Value |
|-----------|----------|-------|-------|-------|
| Node 1 → {4,5} | p(1→4) | 0.60 | p(1→4,5) | 0.30 |
| | p(1→5) | 0.50 | | |
| Node 2 → {5,6} | p(2→5) | 0.50 | p(2→5,6) | 0.30 |
| | p(2→6) | 0.55 | | |
| Node 3 → {5,6} | p(3→5) | 0.45 | p(3→5,6) | 0.25 |
| | p(3→6) | 0.50 | | |

**Simulation parameters:** 10,000 packets per run.

---

## Phase Summary

| Phase | File | Input | Topology | Metric | Key Result |
|-------|------|-------|----------|--------|------------|
| 1 | `phase1_simplex_fd.cpp` | stdin | 0→{1,2}→3 | Analytical | FD: 6.1% latency reduction |
| 2 | `phase2_3fwd_1dest.cpp` | stdin | 0→{1,2,3}→4 | Analytical | Simplex ETX = 3.063 |
| 3 | `phase3_3fwd_1dest_fd.cpp` | stdin | 0→{1,2,3}→4 | Analytical | FD: 9.5% latency reduction |
| 4 | `phase4_3fwd_3dest_simplex.cpp` | stdin | 0→{1,2,3}→{4,5,6} | Sim + Analytical | 5.995 vs 5.990 (0.08% gap) |
| 4a | `phase4_analytical.cpp` | stdin | 0→{1,2,3}→{4,5,6} | Analytical (no RNG) | Bitmask engine: 4.420 slots |
| 5 | `phase5_3fwd_3dest_fd.cpp` | stdin | 0→{1,2,3}→{4,5,6} | Sim + Analytical | FD: 25.9% latency reduction |
| 6 | `phase6_generalized.cpp` | file | Arbitrary graph | Analytical | Generalized multi-hop (WIP) |

---

## Phase 1 — Basic ETX Calculator (2 Forwarders, 1 Destination)

**File:** `phase1_simplex_fd.cpp` · **Input:** stdin (interactive prompts)

### What It Does

Implements the foundational ETX formula for 2 forwarders with correlated links. Given marginal probabilities p(0→1), p(0→2) and joint probability p(0→1,2), computes:

1. **Venn region probabilities** — P(none), P(only1), P(only2), P(both)
2. **Simplex ETX** — one forwarder per slot (sequential)
3. **Full Duplex ETX** — both forwarders transmit simultaneously

### Formulas

**Simplex:**
```
E = (1 + P(only1)·c13 + P(only2)·c23 + P(both)·min(c13,c23)) / (1 - P(none))
```

**Full Duplex (latency):**
```
E = (1 + P(only1)·c13 + P(only2)·c23 + P(both)·(1/p_union)) / (1 - P(none))
```

**Full Duplex (resource):**
```
E = (1 + P(only1)·c13 + P(only2)·c23 + P(both)·(2/p_union)) / (1 - P(none))
```

where `p_union = p13 + p23 - p13·p23`.

### Results (Base Test Case)

| Metric | Simplex | Full Duplex | Change |
|--------|---------|-------------|--------|
| Latency (slots) | 3.389 | 3.181 | **6.1% less** |
| Resource (tx) | 3.389 | 3.806 | 12.3% more |

---

## Phase 2 — 3-Forwarder Simplex ETX (Analytical)

**File:** `phase2_3fwd_1dest.cpp` · **Input:** stdin

### What's New (vs Phase 1)

- Extends to **3 forwarders** with **8 Venn regions** (none, only1, only2, only3, only12, only13, only23, all3)
- Computes ETX for **all 7 non-empty forwarder subsets**
- Reports best forwarder set with minimum cost

### Formula

```
E = (1 + Σ P(region) · X(region)) / (1 - P(none))
```

where `X(region) = min(c14, c24, c34)` depending on which forwarders are active in that region.

### Results (Base Test Case)

| Metric | Simplex |
|--------|---------|
| Latency (slots) | 3.063 |
| Resource (tx) | 3.063 |

---

## Phase 3 — 3-Forwarder Simplex vs Full Duplex (Analytical)

**File:** `phase3_3fwd_1dest_fd.cpp` · **Input:** stdin

### What's New (vs Phase 2)

- Adds **Full Duplex** computation alongside simplex
- For FD, all active forwarders transmit simultaneously per slot
- Reports dual metric: latency (slots) and resource (tx)

### Results (Base Test Case)

| Metric | Simplex | Full Duplex | Change |
|--------|---------|-------------|--------|
| Latency (slots) | 3.063 | 2.771 | **9.5% less** |
| Resource (tx) | 3.063 | 3.709 | 21.1% more |

---

## Phase 4 — Multicast Simplex (Simulation + Analytical)

**Files:** `phase4_3fwd_3dest_simplex.cpp` (simulation), `phase4_analytical.cpp` (bitmask engine)
**Input:** stdin (interactive prompts)

### What's New (vs Phase 3)

- Extends to **multicast delivery** — all 3 destinations {4,5,6} must be covered
- **Monte Carlo simulation** with 10,000 packets and L'Ecuyer RNG
- **Bitmask coverage-state analytical engine** — no RNG, iterative fixed-point over all coverage states
- **Optimal relay selection** — at each coverage state, pick the relay with minimum expected remaining cost

### Network Topology

```
Source (0)  →  Forwarders (1,2,3)  →  Destinations (4,5,6)

Reachability:
  Node 1 → {4, 5}
  Node 2 → {5, 6}
  Node 3 → {5, 6}

Absorbing condition: (f1 received) AND (f2 OR f3 received)
```

### Results (Base Test Case)

| Metric | Simulation | Analytical | Agreement |
|--------|-----------|------------|-----------|
| Simplex slots | 5.995 | 5.990 | **0.08%** |
| Forwarder costs | R1=2.417, R2=2.485, R3=2.794 | — | — |

---

## Phase 5 — Simplex vs Full Duplex (Simulation + Analytical)

**File:** `phase5_3fwd_3dest_fd.cpp` · **Input:** stdin

### What's New (vs Phase 4)

- Runs **both** simplex and full duplex simulations for direct comparison
- Simplex uses **optimal relay selection** (minimum cost per coverage state)
- Full duplex uses **relay deactivation** — relays whose destinations are already covered stop transmitting

### Results (Base Test Case)

| Metric | Simplex (sim) | FD (sim) | FD (theory) | Gap |
|--------|--------------|----------|-------------|-----|
| Latency (slots) | 5.995 | 4.442 | 4.420 | 0.50% |
| Resource (tx) | 5.995 | 6.535 | 6.495 | 0.60% |

### Headline Result

**Full duplex reduces delivery latency by 25.9% at the cost of 9.0% more radio transmissions.**

---

## Phase 6 — Generalized Multi-Hop Network (WIP)

**File:** `phase6_generalized.cpp` · **Input:** `phase6_input.txt`

### What It Does

Reads an arbitrary network topology from an input file and computes routing tables using iterative bottom-up approach. Supports multi-hop paths with configurable edge costs.

### Current Limitations

- **Simplex only** — no full duplex support
- **Independence assumption** — does not model link correlation
- No correlation-aware routing tables

---

## Validation Summary

| Comparison | Simulation | Theory | Error |
|-----------|------------|--------|-------|
| Phase 4 Simplex | 5.995 | 5.990 | 0.08% |
| Phase 5 Simplex | 5.995 | 5.990 | 0.08% |
| Phase 5 FD Latency | 4.442 | 4.420 | 0.50% |
| Phase 5 FD Resource | 6.535 | 6.495 | 0.60% |

All within ~1% sampling noise for 10,000 packets.

---

## How to Build & Run

### Prerequisites

- C++ compiler supporting C++17 or later (e.g., `g++`, MSVC)

### Compile

```bash
g++ -o phase1 phase1_simplex_fd.cpp -std=c++17
g++ -o phase2 phase2_3fwd_1dest.cpp -std=c++17
g++ -o phase3 phase3_3fwd_1dest_fd.cpp -std=c++17
g++ -o phase4 phase4_3fwd_3dest_simplex.cpp -std=c++17
g++ -o phase4a phase4_analytical.cpp -std=c++17
g++ -o phase5 phase5_3fwd_3dest_fd.cpp -std=c++17
g++ -o phase6 phase6_generalized.cpp -std=c++17
```

### Run

```bash
# Phases 1-5 and 4a: enter values interactively
./phase1
./phase2
./phase3
./phase4
./phase4a
./phase5

# Phase 6: reads from phase6_input.txt
./phase6
```

> All files accept runtime input via `cin` with clear prompts.

---

## Input File Format

### Phases 1-3 (stdin, interactive)

Enter probabilities one by one when prompted:

```
Enter p(0->1): 0.5
Enter p(0->2): 0.4
Enter p(0->1 AND 0->2): 0.3
...
```

### Phases 4-5, 4a (stdin, multi-line)

Enter all values in sequence:

```
0.5 0.4 0.45          ← hop-1 marginals p(0→1) p(0→2) p(0→3)
0.3 0.25 0.2 0.15     ← hop-1 joints p(0→1,2) p(0→1,3) p(0→2,3) p(0→1,2,3)
0.6 0.5 0.3           ← hop-2: node 1→{4,5} p(1→4) p(1→5) p(1→4,5)
0.5 0.55 0.3          ← hop-2: node 2→{5,6} p(2→5) p(2→6) p(2→5,6)
0.45 0.5 0.25         ← hop-2: node 3→{5,6} p(3→5) p(3→6) p(3→5,6)
10000                 ← number of packets
```

### Phase 6 (`phase6_input.txt`)

```
# Number of nodes
7
# Source node
0
# Number of destinations
3
# Destination nodes
4 5 6
# Number of edges
9
# Edges (from to cost)
0 1 2.0
0 2 2.5
...
```

---

## Project Structure

```
Link_Correlation/
├── phase1_simplex_fd.cpp        Phase 1: Basic 2-fwd ETX (analytical)
├── phase2_3fwd_1dest.cpp        Phase 2: 3-fwd simplex ETX (analytical)
├── phase3_3fwd_1dest_fd.cpp     Phase 3: Simplex vs FD (analytical)
├── phase4_3fwd_3dest_simplex.cpp Phase 4: Multicast simplex (simulation)
├── phase4_analytical.cpp        Phase 4: Bitmask engine (analytical, no RNG)
├── phase5_3fwd_3dest_fd.cpp     Phase 5: Simplex vs FD (simulation)
├── phase6_generalized.cpp       Phase 6: Generalized multi-hop (WIP)
├── phase6_input.txt             Sample input for Phase 6
├── results_full_duplex.md       Complete results with all validated numbers
├── 3Forwarders_1dest.cpp        Legacy: 3-fwd 1-dest analytical
├── 3Forwarders_2dest.cpp        Legacy: 3-fwd 2-dest analytical
├── etx_case_study.cpp           Legacy: Full implementation
├── simulation_basic.cpp         Legacy: Basic Monte Carlo simulator
└── README.md                    This file
```

---

## Tech Stack

- **Language:** C++17 (STL)
- **RNG:** L'Ecuyer combined multiplicative congruential generator (Phases 4-5)
- **Concepts:** Probability Theory, Graph Routing, Wireless Networks, Simulation Modeling, Markov Chains, Fixed-Point Iteration
