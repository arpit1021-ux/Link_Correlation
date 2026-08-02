// Phase 1: Basic ETX Calculator — 2 Forwarders, 1 Destination
// Topology: 0 -> {1, 2} -> {3}
// Computes Simplex ETX (sequential) vs Full Duplex ETX (parallel)
// Reports both metrics: Latency (time slots) and Resource (total transmissions)
//
// Based on M-LCAR paper (Rathore, Dhaka, Bose, IIT Guwahati)
// Paper's Eq. (1): cEAX_{S,DS}(f_s)
//
// DERIVATION:
// E = expected transmissions from current state
// E = 1 + P(none)*E + P(only1)*c13 + P(only2)*c23 + P(both)*X
// => E = (1 + P(only1)*c13 + P(only2)*c23 + P(both)*X) / (1 - P(none))
// where X depends on simplex vs full duplex:
//   Simplex: X = min(c13, c23)       [pick best forwarder]
//   FD latency: X = 1/p_union        [both transmit, dest receives if either succeeds]
//   FD resource: X = 2/p_union        [2 transmissions per slot × expected slots]

#include <bits/stdc++.h>
using namespace std;

const double INF = 1e12;
const double EPS = 1e-12;

struct RegionProbs {
    double none, only1, only2, both;
};

RegionProbs computeRegions(double p01, double p02, double p012) {
    RegionProbs r;
    r.only1 = max(0.0, p01 - p012);
    r.only2 = max(0.0, p02 - p012);
    r.both  = max(0.0, p012);
    r.none  = max(0.0, 1.0 - p01 - p02 + p012);
    return r;
}

double linkETX(double p) {
    return (p > EPS) ? 1.0 / p : INF;
}

struct DualMetric {
    double latency;   // time slots
    double resource;  // total transmissions
};

// E = (1 + P(only1)*c13 + P(only2)*c23 + P(both)*X) / (1 - P(none))
// Simplex: X = min(c13, c23)
// Since latency == resource for simplex (one tx per slot):
DualMetric computeSimplexETX(double p01, double p02, double p012,
                              double p13, double p23) {
    RegionProbs reg = computeRegions(p01, p02, p012);
    double c13 = linkETX(p13);
    double c23 = linkETX(p23);
    double denom = 1.0 - reg.none;
    if (denom < EPS) return {INF, INF};

    double X = min(c13, c23);
    double etx = (1.0 + reg.only1 * c13 + reg.only2 * c23 + reg.both * X) / denom;
    return {etx, etx};
}

// Full Duplex: all active forwarders transmit simultaneously
// FD latency: X = 1/p_union (expected slots when both transmit)
// FD resource: X = 2/p_union (2 tx per slot × expected slots)
DualMetric computeFullDuplexETX(double p01, double p02, double p012,
                                 double p13, double p23) {
    RegionProbs reg = computeRegions(p01, p02, p012);
    double c13 = linkETX(p13);
    double c23 = linkETX(p23);
    double denom = 1.0 - reg.none;
    if (denom < EPS) return {INF, INF};

    // P(dest receives from at least one forwarder) = p13 + p23 - p13*p23 (independent)
    double p_union = p13 + p23 - p13 * p23;
    double fd_both_slots = (p_union > EPS) ? 1.0 / p_union : INF;

    // Latency: X = expected slots for "both" case
    double etx_lat = (1.0 + reg.only1 * c13 + reg.only2 * c23 + reg.both * fd_both_slots) / denom;

    // Resource: X = 2 transmissions per slot × expected slots
    double etx_res = (1.0 + reg.only1 * c13 + reg.only2 * c23 + reg.both * 2.0 * fd_both_slots) / denom;

    return {etx_lat, etx_res};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "========================================================\n";
    cout << "  Phase 1: ETX Calculator — 2 Forwarders, 1 Destination\n";
    cout << "  Topology: 0 -> {1, 2} -> {3}\n";
    cout << "  Simplex vs Full Duplex (Dual Metric)\n";
    cout << "========================================================\n\n";

    double p01, p02, p012, p13, p23;
    cout << "Enter p(0->1): ";          cin >> p01;
    cout << "Enter p(0->2): ";          cin >> p02;
    cout << "Enter p(0->1 AND 0->2): "; cin >> p012;
    cout << "Enter p(1->3): ";          cin >> p13;
    cout << "Enter p(2->3): ";          cin >> p23;

    if (p012 > min(p01, p02) + EPS || p012 < max(0.0, p01 + p02 - 1.0) - EPS)
        cerr << "Warning: p012 may be outside valid bounds.\n";

    RegionProbs reg = computeRegions(p01, p02, p012);
    double c13 = linkETX(p13), c23 = linkETX(p23);

    cout << "\n--- Region Probabilities ---\n";
    cout << "P(none)=" << reg.none << " P(only1)=" << reg.only1
         << " P(only2)=" << reg.only2 << " P(both)=" << reg.both << "\n";

    cout << "\n--- Link Costs ---\n";
    cout << "ETX(1->3) = " << c13 << "  ETX(2->3) = " << c23 << "\n";

    DualMetric simplex = computeSimplexETX(p01, p02, p012, p13, p23);
    DualMetric fd = computeFullDuplexETX(p01, p02, p012, p13, p23);

    cout << "\n========================================================\n";
    cout << "  RESULTS\n";
    cout << "========================================================\n";
    cout << "\nSimplex (one forwarder per slot):\n";
    cout << "  Latency  (slots) = " << simplex.latency << "\n";
    cout << "  Resource (tx)    = " << simplex.resource << "\n";

    cout << "\nFull Duplex (all forwarders per slot):\n";
    cout << "  Latency  (slots) = " << fd.latency << "\n";
    cout << "  Resource (tx)    = " << fd.resource << "\n";

    if (simplex.latency > EPS && fd.latency > EPS) {
        double latRed = (simplex.latency - fd.latency) / simplex.latency * 100.0;
        double resInc = (fd.resource - simplex.resource) / simplex.resource * 100.0;
        cout << "\n--- Comparison ---\n";
        cout << "                    Simplex    Full Duplex  Change\n";
        cout << "Latency  (slots)  " << setw(10) << simplex.latency
             << "  " << setw(10) << fd.latency
             << "  " << latRed << "% less\n";
        cout << "Resource (tx)     " << setw(10) << simplex.resource
             << "  " << setw(10) << fd.resource
             << "  " << resInc << "% more\n";
    }

    cout << "\n========================================================\n";
    return 0;
}
