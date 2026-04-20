# Link Correlation Aware Routing (BTP Project)

This project focuses on analyzing and implementing **correlation-aware opportunistic routing** in wireless multihop networks. It extends traditional ETX-based routing by incorporating **joint reception probabilities** to model realistic wireless behavior.

---

## 📌 Overview

Wireless networks suffer from unreliable links due to interference, fading, and shared medium effects. Traditional routing assumes independent link behavior, which is often inaccurate.

This project addresses this limitation by:
- Modeling **correlated packet receptions**
- Using **Expected Transmission Count (ETX)** as a routing metric
- Extending ETX to **forwarder sets and multicast routing**
- Validating analytical results using **simulation**

---

## ⚙️ Features

- 📊 Analytical ETX computation for:
  - Single destination routing
  - Multicast routing
- 🔗 Correlation-aware probability modeling
- 🧠 Forwarder set selection using **heuristic (minimum ETX)**
- 🎲 Simulation using random number generation
- 📈 Comparison of **Analytical vs Simulated ETX**

---

## 🧪 Simulation Details

- Packet transmissions are simulated using probabilistic models
- Random number generation is used to emulate wireless link behavior
- Multi-stage process:
  1. Source → Forwarders
  2. Forwarders → Destinations
- Simulation repeated for large number of packets (e.g., 10,000)

---

## 🛠️ Tech Stack

- **Language:** C++
- **Libraries:** STL, <random>
- **Concepts Used:**
  - Probability Theory
  - Graph Routing
  - Wireless Networks
  - Simulation Modeling

---

## 🚀 Future Scope

- Extend to larger network topologies
- Real-time link probability estimation
- Integration with network simulators (NS-3, OMNeT++)
- Machine learning-based forwarder selection

---