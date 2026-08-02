# Full Duplex Transmission — Final Results

## Project: Link-Correlation Aware Routing in Multihop Wireless Networks

### Innovation: Full Duplex Transmission Model
Multiple forwarders transmit simultaneously in the same time slot, reducing latency at the cost of increased radio resource usage.

---

## Base Test Case

**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Hop-1 | Value | Hop-2 | Value |
|-------|-------|-------|-------|
| p(0→1) | 0.50 | p(1→4) | 0.60 |
| p(0→2) | 0.40 | p(1→5) | 0.50 |
| p(0→3) | 0.45 | p(1→4,5) | 0.30 |
| p(0→1,2) | 0.30 | p(2→5) | 0.50 |
| p(0→1,3) | 0.25 | p(2→6) | 0.55 |
| p(0→2,3) | 0.20 | p(2→5,6) | 0.30 |
| p(0→1,2,3) | 0.15 | p(3→5) | 0.45 |
| | | p(3→6) | 0.50 |
| | | p(3→5,6) | 0.25 |

---

## Phase 1 — Basic ETX Calculator

**Topology:** 0 → {1, 2} → {3}

| Metric | Simplex | Full Duplex | Change |
|--------|---------|-------------|--------|
| Latency (slots) | 3.389 | 3.181 | **6.1% less** |
| Resource (tx) | 3.389 | 3.806 | 12.3% more |

---

## Phase 2 — 3-Forwarder Simplex ETX

**Topology:** 0 → {1, 2, 3} → {4}

| Metric | Simplex |
|--------|---------|
| Latency (slots) | 3.063 |
| Resource (tx) | 3.063 |

---

## Phase 3 — 3-Forwarder Simplex vs Full Duplex

**Topology:** 0 → {1, 2, 3} → {4}

| Metric | Simplex | Full Duplex | Change |
|--------|---------|-------------|--------|
| Latency (slots) | 3.063 | 2.771 | **9.5% less** |
| Resource (tx) | 3.063 | 3.709 | 21.1% more |

---

## Phase 4 — Multicast Simplex

**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Metric | Simulation | Analytical | Agreement |
|--------|-----------|------------|-----------|
| Simplex slots | 5.995 | 5.990 | **0.08%** |
| Forwarder costs | R1=2.417, R2=2.485, R3=2.794 | — | — |

---

## Phase 4 Analytical — Bitmask Engine

**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Metric | Full Duplex | Simplex |
|--------|------------|---------|
| Slots | 4.4198 | 5.9902 |
| TX/resource | 6.4947 | 5.9902 |

---

## Phase 5 — Simplex vs Full Duplex

**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Metric | Simplex (sim) | FD (sim) | FD (theory) | Gap |
|--------|--------------|----------|-------------|-----|
| Latency (slots) | 5.995 | 4.442 | 4.420 | 0.50% |
| Resource (tx) | 5.995 | 6.535 | 6.495 | 0.60% |

### Headline Result
**Full duplex reduces delivery latency by 25.9% at the cost of 9.0% more radio transmissions.**

---

## Validation Summary

| Comparison | Simulation | Theory | Error |
|-----------|------------|--------|-------|
| Phase 4 Simplex | 5.995 | 5.990 | 0.08% |
| Phase 5 Simplex | 5.995 | 5.990 | 0.08% |
| Phase 5 FD Latency | 4.442 | 4.420 | 0.50% |
| Phase 5 FD Resource | 6.535 | 6.495 | 0.60% |

All within ~1% sampling noise for 10,000 packets.
