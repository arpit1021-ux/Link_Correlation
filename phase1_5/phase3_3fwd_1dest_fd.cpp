// Phase 3: 3-Forwarder Full Duplex ETX — 1 Destination
// Topology: 0 -> {1, 2, 3} -> {4}
// Full Duplex: all active forwarders transmit simultaneously
// Reports BOTH metrics: Latency and Resource
// Shows comparison table for all 7 forwarder subsets
//
// Formula: E = (1 + Σ P(region) * X(region)) / (1 - P(none))
// FD latency: X = 1/p_union (expected slots for simultaneous transmission)
// FD resource: X = N_fwd/p_union (N forwarders × expected slots)

#include <bits/stdc++.h>
using namespace std;

const double INF = 1e12;
const double EPS = 1e-12;

struct RegionProbs3 {
    double none, only1, only2, only3, only12, only13, only23, all3;
};

RegionProbs3 computeRegions3(double p1, double p2, double p3,
                              double p12, double p13, double p23, double p123) {
    RegionProbs3 r;
    r.only1  = max(0.0, p1 - p12 - p13 + p123);
    r.only2  = max(0.0, p2 - p12 - p23 + p123);
    r.only3  = max(0.0, p3 - p13 - p23 + p123);
    r.only12 = max(0.0, p12 - p123);
    r.only13 = max(0.0, p13 - p123);
    r.only23 = max(0.0, p23 - p123);
    r.all3   = max(0.0, p123);
    r.none   = max(0.0, 1.0 - (p1+p2+p3) + (p12+p13+p23) - p123);
    return r;
}

double linkETX(double p) { return (p > EPS) ? 1.0 / p : INF; }

struct DualMetric {
    double latency, resource;
};

// Compute union probability: P(at least one succeeds) for independent links
double pUnion(double a, double b) { return a + b - a * b; }
double pUnion3(double a, double b, double c) {
    return 1.0 - (1.0 - a) * (1.0 - b) * (1.0 - c);
}

// Simplex ETX (for comparison)
DualMetric computeSimplexETX3(double p1, double p2, double p3,
                               double p12, double p13, double p23, double p123,
                               double p14, double p24, double p34) {
    RegionProbs3 reg = computeRegions3(p1, p2, p3, p12, p13, p23, p123);
    double c14 = linkETX(p14), c24 = linkETX(p24), c34 = linkETX(p34);
    double denom = 1.0 - reg.none;
    if (denom < EPS) return {INF, INF};

    double E = 1.0
        + reg.only1  * c14
        + reg.only2  * c24
        + reg.only3  * c34
        + reg.only12 * min(c14, c24)
        + reg.only13 * min(c14, c34)
        + reg.only23 * min(c24, c34)
        + reg.all3   * min({c14, c24, c34});

    double etx = E / denom;
    return {etx, etx};
}

// Full Duplex ETX
DualMetric computeFullDuplexETX3(double p1, double p2, double p3,
                                  double p12, double p13, double p23, double p123,
                                  double p14, double p24, double p34) {
    RegionProbs3 reg = computeRegions3(p1, p2, p3, p12, p13, p23, p123);
    double c14 = linkETX(p14), c24 = linkETX(p24), c34 = linkETX(p34);
    double denom = 1.0 - reg.none;
    if (denom < EPS) return {INF, INF};

    // Expected slots for each region (FD: all transmit simultaneously)
    double X_lat_only1  = c14;                                  // 1 fwd
    double X_lat_only2  = c24;                                  // 1 fwd
    double X_lat_only3  = c34;                                  // 1 fwd
    double X_lat_only12 = 1.0 / pUnion(p14, p24);              // 2 fws parallel
    double X_lat_only13 = 1.0 / pUnion(p14, p34);              // 2 fws parallel
    double X_lat_only23 = 1.0 / pUnion(p24, p34);              // 2 fws parallel
    double X_lat_all3   = 1.0 / pUnion3(p14, p24, p34);        // 3 fws parallel

    // Latency ETX
    double E_lat = 1.0
        + reg.only1  * X_lat_only1
        + reg.only2  * X_lat_only2
        + reg.only3  * X_lat_only3
        + reg.only12 * X_lat_only12
        + reg.only13 * X_lat_only13
        + reg.only23 * X_lat_only23
        + reg.all3   * X_lat_all3;

    // Resource ETX: transmissions = N_fwd × expected_slots
    double X_res_only1  = 1 * c14;
    double X_res_only2  = 1 * c24;
    double X_res_only3  = 1 * c34;
    double X_res_only12 = 2 * X_lat_only12;
    double X_res_only13 = 2 * X_lat_only13;
    double X_res_only23 = 2 * X_lat_only23;
    double X_res_all3   = 3 * X_lat_all3;

    double E_res = 1.0
        + reg.only1  * X_res_only1
        + reg.only2  * X_res_only2
        + reg.only3  * X_res_only3
        + reg.only12 * X_res_only12
        + reg.only13 * X_res_only13
        + reg.only23 * X_res_only23
        + reg.all3   * X_res_all3;

    return {E_lat / denom, E_res / denom};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "============================================================\n";
    cout << "  Phase 3: 3-Forwarder Simplex vs Full Duplex ETX\n";
    cout << "  Topology: 0 -> {1, 2, 3} -> {4}\n";
    cout << "============================================================\n\n";

    double p01, p02, p03, p012, p013, p023, p0123;
    double p14, p24, p34;

    cout << "Enter p(0->1) p(0->2) p(0->3): ";
    cin >> p01 >> p02 >> p03;
    cout << "Enter p(0->1,2) p(0->1,3) p(0->2,3) p(0->1,2,3): ";
    cin >> p012 >> p013 >> p023 >> p0123;
    cout << "Enter p(1->4) p(2->4) p(3->4): ";
    cin >> p14 >> p24 >> p34;

    RegionProbs3 reg = computeRegions3(p01, p02, p03, p012, p013, p023, p0123);
    double c14 = linkETX(p14), c24 = linkETX(p24), c34 = linkETX(p34);

    cout << "\n--- Region Probabilities ---\n";
    cout << "none=" << reg.none << " only1=" << reg.only1 << " only2=" << reg.only2
         << " only3=" << reg.only3 << "\n";
    cout << "only12=" << reg.only12 << " only13=" << reg.only13
         << " only23=" << reg.only23 << " all3=" << reg.all3 << "\n";

    DualMetric simplex = computeSimplexETX3(p01, p02, p03, p012, p013, p023, p0123, p14, p24, p34);
    DualMetric fd = computeFullDuplexETX3(p01, p02, p03, p012, p013, p023, p0123, p14, p24, p34);

    cout << "\n============================================================\n";
    cout << "  RESULTS: Simplex vs Full Duplex\n";
    cout << "============================================================\n";

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

    cout << "\n============================================================\n";
    return 0;
}
