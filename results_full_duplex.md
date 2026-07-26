# Full Duplex Transmission — Final Results

## Project: Link-Correlation Aware Routing in Multihop Wireless Networks

### Innovation: Full Duplex Transmission Model
Multiple forwarders transmit simultaneously in the same time slot, reducing latency at the cost of increased radio resource usage.

---

## Phase 1: Basic ETX Calculator (2 forwarders, 1 destination)
**Topology:** 0 → {1, 2} → {3}

| Metric | Simplex | Full Duplex | Change |
|--------|---------|-------------|--------|
| Latency (slots) | 3.389 | 3.181 | **6.1% less** |
| Resource (tx) | 3.389 | 3.806 | 12.3% more |

**Input:** p01=0.5, p02=0.4, p012=0.3, p13=0.6, p23=0.5
**Validation:** Analytical and simulation match within 0.2%

---

## Phase 2: 3-Forwarder Simplex ETX (3 forwarders, 1 destination)
**Topology:** 0 → {1, 2, 3} → {4}

| Metric | Simplex |
|--------|---------|
| Latency (slots) | 3.063 |
| Resource (tx) | 3.063 |

---

## Phase 3: 3-Forwarder Full Duplex ETX (3 forwarders, 1 destination)
**Topology:** 0 → {1, 2, 3} → {4}

| Metric | Simplex | Full Duplex | Change |
|--------|---------|-------------|--------|
| Latency (slots) | 3.063 | 2.771 | **9.5% less** |
| Resource (tx) | 3.063 | 3.709 | 21.1% more |

---

## Phase 3.5: Paper Topology Validation
**Topology:** S → f1 → D1, S → f2 → D2, S → f3 → {D1, D2}

All checkpoints passed against published anchor:
- ETX(f3 → {D1,D2}) = 2.0833 (expected 2.0834)
- E0 (from ∅) = 3.0240 (expected 3.024)
- Best relay-set: {f1,f2,f3} (downstream cost = 1.2544)

---

## Phase 4: Multicast Simulation — Simplex (3 forwarders, 3 destinations)
**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Metric | Simulation | Analytical | Agreement |
|--------|-----------|------------|-----------|
| Simplex slots | **5.995** | **5.990** | **0.08%** |

**Forwarder costs:** R1=2.417, R2=2.485, R3=2.794
**Relay selection:** Optimal (minimum-cost relay per coverage state)

---

## Phase 4 Analytical: Bitmask Engine (No RNG)
**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Metric | Full Duplex | Simplex |
|--------|------------|---------|
| Slots | 4.4198 | 5.9902 |
| TX/resource | 6.4947 | 5.9902 |

---

## Phase 5: Multicast Simulation — Simplex vs Full Duplex
**Topology:** 0 → {1, 2, 3} → {4, 5, 6}

| Metric | Simplex (sim) | FD (sim) | FD analytical | Gap |
|--------|--------------|----------|---------------|-----|
| Latency (slots) | 5.995 | 4.442 | 4.420 | 0.50% |
| Resource (tx) | 5.995 | 6.535 | 6.495 | 0.60% |

### Key BTP Result
**Full duplex reduces delivery latency by 25.9% at the cost of 9.0% more radio transmissions.**

---

## Validation Summary

| Comparison | Sim | Theory | Error |
|-----------|-----|--------|-------|
| Phase 4 Simplex | 5.995 | 5.990 | 0.08% |
| Phase 5 Simplex | 5.995 | 5.990 | 0.08% |
| Phase 5 FD Latency | 4.442 | 4.420 | 0.50% |
| Phase 5 FD Resource | 6.535 | 6.495 | 0.60% |
| Phase 3.5 Anchor | 3.024 | 3.024 | 0.00% |

All within ~1% sampling noise for 10,000 packets.

---

## Bug Fixes Applied (17+ bugs found and fixed)
1. **R1/R2/R3 formula:** `(r.none + r.oA*(1+cA) + ...) / (1-r.none)` → `(1 + r.oA*cB + r.oB*cA) / (1-r.none)`
2. **Simplex relay selection:** Round-robin (`fwdList[fwdIdx % size]`) → Optimal relay selection (minimum `relayCostAtState()` per coverage state)
3. **FD relay deactivation:** `fwdList` computed once after hop-1 → Recomputed each round based on current coverage
4. **Phase 4 Analytical:** Hardcoded values → Runtime input via `cin`
5. **Phase 4 Analytical Etx step cost:** Hardcoded `acc=1.0` → Active relay count per state

## Files
All files accept runtime input via `cin`.
