# HOW TO USE `phase6_simplex_combined.cpp`

This ONE program takes ANY acyclic multi-hop topology as input - no
topology is hardcoded. It computes the analytical routing table, runs a
matching simulation, and prints a final comparison, all in one run.

## BUILD

```bash
g++ -O2 -std=c++17 -o phase6_simplex_combined phase6_simplex_combined.cpp
```

## RUN

```bash
./phase6_simplex_combined < your_topology.txt
```

(or type values interactively when prompted - the prompts tell you
exactly what to enter at each step)

## INPUT FORMAT (plain numbers only, no comments - cin cannot skip '#' lines)

**Line 1:**

```
N S
```

- `N` = number of nodes (ids `0..N-1`)
- `S` = source node id

**Line 2:**

```
D
```

- number of destinations (max 4 supported)

**Line 3:**

```
d1 d2 ... dD
```

- the `D` destination node ids

**Next N blocks (one per node, in order `0..N-1`):**

```
k
n1 c1 n2 c2 ... nk ck
```

- `k` = out-degree of this node (`0` if it's a destination/sink)
- then `k` pairs: `(neighbor id, ETX cost of that link)`

**Next blocks (one per node with out-degree >= 2, in the SAME order, skipping any node whose out-degree is 0 or 1):**

```
C
```

then `C` lines, each:

```
m id_1 ... id_m v
```

- `m` = size of a correlated group of this node's own links
- `id_1..id_m` = which neighbor ids are in that group
- `v` = `P(all m of those links succeed together)`

`C = 0` means: treat every pair among this node's links as independent - a valid, simpler default.

**Last line:**

```
targetPackets
```

How many independent packets to simulate (no queueing - each packet is simulated fully, one at a time, matching what the analytical model actually represents: a single packet's expected journey, not continuous traffic)

## VALIDATION THE PROGRAM DOES FOR YOU

- Rejects non-positive costs or probabilities > 1
- Rejects correlation values outside [0,1]
- Warns if a node's full outcome-probability distribution doesn't sum to 1.0 (a sign the given correlation numbers are inconsistent)
- Warns if any computed probability comes out negative (a Frechet-bound violation in the given correlation data)
- Warns if analytical vs simulated results differ by more than 5%, since every validated case in this project has landed well under that

## FIVE WORKED EXAMPLES INCLUDED (`phase6_test_cases/`)

### `test1_phase1_2fwd_1dest.txt`

Phase 1's original 2-forwarder, 1-destination case.

**Expected:** `3.3889`

(matches the hand-derived `3.388889` from the very start of this project)

### `test2_phase2style_3fwd_1dest.txt`

Phase 2/3's 3-forwarder, 1-destination case.

**Expected:** `3.0626`

(matches the hand-derived `3.062626` exactly)

### `test3_phase4style_3fwd_3dest.txt`

Phase 4/5's original topology: 3 forwarders, 3 destinations, where one destination is ONLY reachable via one specific forwarder and another ONLY via a different one - genuinely requires accumulating multiple distinct relay branches, not just a single chain.

**Expected:** `~6.42`

### `test4_new_3hop_chain.txt`

A new, previously-untested 3-hop topology (`0->{1,2}->3->{4,5}`).

**Expected:** `~5.61`

### `test5_own16node_multihop.txt`

The validated 16-node network.

**Expected:** `6.8336`

## WHAT'S INTENTIONALLY NOT YET SUPPORTED

- Cycles in the topology (every edge should go toward the destinations; a genuinely cyclic mesh needs a further, separate validation pass, the way Fig. 2's `9<->13` cycle case did during development)
- Full duplex (this file is SIMPLEX ONLY - a single relay/path continues after a broadcast; full duplex racing is a deliberately separate, not-yet-finished piece of work with a known open issue in its resource metric - do not merge it in until that's resolved)