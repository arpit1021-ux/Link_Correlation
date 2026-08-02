# Link-Correlation Aware Routing in Multihop Wireless Networks 

This project analyzes and implements **correlation-aware opportunistic routing**
in wireless multihop networks. It extends traditional ETX-based routing by
incorporating **joint (correlated) reception probabilities** instead of
assuming independent link behavior, and studies the tradeoff between
**simplex** (single relay continues at a time) and **full-duplex**
(multiple correlated forwarders transmit concurrently) forwarding strategies.

---

## 📌 Overview

Wireless links fail in correlated ways — shared interference, fading, and
medium contention mean nearby links often succeed or fail together, not
independently. Traditional ETX-based routing ignores this. This project:

- Models **correlated packet reception** using joint probability data
- Computes **Expected Transmission Count (ETX)** as a routing cost metric,
  generalized to both single-destination and multicast delivery
- Extends this to **forwarder-set selection** and full multi-hop routing
  tables, solved via fixed-point iteration (not hand-derived formulas)
- Characterizes the **simplex vs. full-duplex tradeoff**: full duplex
  reduces delivery latency but increases total transmission cost — both
  measured and reported explicitly, not just one
- Validates every analytical result against an independent discrete-event
  simulation, for every phase of the project

---

## ⚙️ Project Structure

Link_Correlation/
├── legacy/                  <- mini project work
├── phase1_5/                <- the corrected/validated Phase 1-5 files
│   ├── phase1_simplex_fd.cpp
│   ├── phase2_3fwd_1dest.cpp
│   ├── phase3_3fwd_1dest_fd.cpp
│   ├── phase4_3dest_sim.cpp
│   └── phase5_3dest_fd_sim.cpp
├── phase6/
│   ├── phase6_simplex_combined.cpp
│   ├── test_cases/
│   │   ├── test1_phase1_2fwd_1dest.txt
│   │   ├── test2_phase2style_3fwd_1dest.txt
│   │   ├── test3_phase4style_3fwd_3dest.txt
│   │   ├── test4_new_3hop_chain.txt
│   │   └── test5_own16node_multihop.txt
│   └── README.md            <- input format explanation
└── README.md                <- top-level, updated (see below)

---

## 🧪 Methodology

- **Analytical model:** per-node expected-cost equations solved via
  fixed-point iteration, using exact inclusion-exclusion over correlated
  reception outcomes (no independence assumption)
- **Simulation:** discrete-event, one packet at a time, using the same
  correlated reception model, for direct comparison against the analytical
  result
- **Validation discipline:** every phase's analytical and simulated results
  are checked against each other before being trusted; several real bugs
  were found and fixed this way during development (see commit history)

---

## 📊 Key Results So Far

| | Latency (slots) | Resource (total transmissions) |
|---|---|---|
| Full duplex vs. simplex | **9–40% lower** | **14–25% higher** |

Full duplex is not a free win — it trades additional transmission/energy
cost for reduced delivery latency. Both effects are measured and reported
together throughout this project.

The generalized Phase 6 engine has been validated across five structurally
different topologies (varying hop depth, forwarder count, and destination
count), with analytical-vs-simulated agreement within **~1%** in every case.

---

## 🛠️ Tech Stack

- **Language:** C++ (STL only, no external dependencies)
- **Concepts used:** probability theory (inclusion-exclusion, Fréchet
  bounds), fixed-point/Bellman-Ford-style iterative solving, discrete-event
  simulation, graph routing

---

## 🚀 Next Steps

- Full-duplex extension of the generalized Phase 6 engine
- Scaling to larger (16–20+ node) topologies
- Routing-table and forwarder-set-selection heuristics for higher-degree
  nodes, where exhaustive forwarding-set search becomes intractable