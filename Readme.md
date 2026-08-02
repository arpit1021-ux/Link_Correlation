# Project Progression

## Evolution from Phases 1–5

The current implementation builds upon the work completed during Phases 1–5 and incorporates several correctness improvements, model refinements, and extensive validation efforts.

### Corrections and Model Improvements

* Corrected three implementation issues identified in the earlier codebase:

  * Updated the analytical formulation in `first_updated.cpp` by correcting the mislabeled probability expression ("both" to "at least one").
  * Replaced the heuristic maximum-based forwarding model in `3Forwarders_1dest.cpp` with a proper union-probability formulation.
  * Verified that the forwarding condition used in `simulation_3dest.cpp`, although initially appearing hardcoded, was structurally correct once multicast delivery semantics were properly interpreted.

### Simplex vs. Full Duplex Evaluation

* Established and quantified the tradeoff between Simplex and Full Duplex forwarding.
* Experimental evaluation showed that Full Duplex consistently reduced end-to-end latency (measured in transmission slots) by approximately **9–40%**, while increasing the total number of transmissions (resource consumption) by approximately **14–25%**.
* These results demonstrate that Full Duplex represents a measurable latency-resource tradeoff rather than a universally superior forwarding strategy.

### Multicast Semantics

* Formalized multicast routing semantics by defining successful delivery as reaching **all destination nodes**, rather than any single destination.
* The analytical model and simulations were updated accordingly, ensuring that the source successfully reaches the required relay combinations necessary for complete multicast coverage.

### Simulation Validation

Several subtle implementation issues were identified and resolved during the validation process, including:

* Event batching order dependence.
* Stale duplicate packet propagation.
* Incorrect accumulation logic caused by overwriting state instead of performing logical union operations.
* Additional synchronization and consistency issues affecting agreement between analytical and simulated results.

Each correction was verified by requiring close agreement between the analytical model and Monte Carlo simulation, rather than relying solely on qualitative correctness.

---

# Phase 6: Generalized Routing Framework

Phase 6 extends the project from topology-specific implementations to a fully generalized routing framework.

## Generalized Analytical and Simulation Engine

The Phase 6 implementation accepts **any acyclic multi-hop topology** as runtime input, including:

* Number of nodes
* Source node
* Destination nodes
* Network connectivity
* Link ETX values
* Correlation information

No network topology is hardcoded, allowing the same implementation to evaluate arbitrary supported networks without modification.

## Validation Across Multiple Network Topologies

The generalized framework was validated on five structurally different test cases, including:

* The original Phase 1 two-forwarder topology.
* The original Phase 2/3 three-forwarder topology.
* The original Phase 4/5 multicast topology.
* A newly designed three-hop multicast network.
* A 16-node multi-hop network.

Across all evaluated cases, the analytical model and Monte Carlo simulation exhibited agreement within approximately **0.06%–0.96%**, providing strong validation of the generalized implementation.

## Dynamic Forwarding Strategy

The generalized routing engine implements a dynamic forwarding methodology in which forwarding decisions are determined according to the evolving packet state throughout the routing process, rather than relying on fixed relay assignments. This enables the framework to support generalized multicast routing over arbitrary acyclic network topologies while maintaining consistency between the analytical model and simulation.
