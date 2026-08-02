// Phase 2: 3-Forwarder Simplex ETX — 1 Destination
// Topology: 0 -> {1, 2, 3} -> {4}
// Computes ETX for all 7 non-empty forwarder subsets
// Simplex model: one forwarder per slot (sequential)
// Reports BOTH metrics: Latency and Resource
//
// Formula: E = (1 + Σ P(region) * X(region)) / (1 - P(none))
// where X(region) = remaining cost for that forwarder subset

#include <bits/stdc++.h>
using namespace std;

const double INF = 1e12;
const double EPS = 1e-12;

struct DualMetric {
    double latency, resource;
};

// ====================== 3-Forwarder Venn Region Probabilities ======================
// 8 regions: none, only1, only2, only3, only12, only13, only23, all3
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

// ====================== Simplex ETX for 3 Forwarders ======================
// For each Venn region, compute the remaining cost X:
// - "only1": only fwd 1 has packet → X = c14
// - "only12": fwds 1,2 have packet → X = min(c14, c24) [simplex: pick best]
// - "all3": all 3 have packet → X = min(c14, c24, c34) [simplex: pick best]
// - etc.
//
// E = (1 + Σ P(region) * X(region)) / (1 - P(none))

DualMetric computeSimplexETX3(double p01, double p02, double p03,
                               double p012, double p013, double p023, double p0123,
                               double p14, double p24, double p34) {
    RegionProbs3 reg = computeRegions3(p01, p02, p03, p012, p013, p023, p0123);
    double c14 = linkETX(p14), c24 = linkETX(p24), c34 = linkETX(p34);
    double denom = 1.0 - reg.none;
    if (denom < EPS) return {INF, INF};

    // Remaining cost for each region (simplex: pick best forwarder)
    double X_only1  = c14;
    double X_only2  = c24;
    double X_only3  = c34;
    double X_only12 = min(c14, c24);
    double X_only13 = min(c14, c34);
    double X_only23 = min(c24, c34);
    double X_all3   = min({c14, c24, c34});

    double E = 1.0
        + reg.only1  * X_only1
        + reg.only2  * X_only2
        + reg.only3  * X_only3
        + reg.only12 * X_only12
        + reg.only13 * X_only13
        + reg.only23 * X_only23
        + reg.all3   * X_all3;

    double etx = E / denom;
    return {etx, etx};  // simplex: latency == resource
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "============================================================\n";
    cout << "  Phase 2: 3-Forwarder Simplex ETX — 1 Destination\n";
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
    cout << "Sum = " << (reg.none + reg.only1 + reg.only2 + reg.only3 +
                          reg.only12 + reg.only13 + reg.only23 + reg.all3) << "\n";

    cout << "\n--- Link Costs ---\n";
    cout << "ETX(1->4)=" << c14 << " ETX(2->4)=" << c24 << " ETX(3->4)=" << c34 << "\n";

    DualMetric result = computeSimplexETX3(p01, p02, p03, p012, p013, p023, p0123, p14, p24, p34);

    cout << "\n============================================================\n";
    cout << "  RESULT: Simplex ETX (0 -> {1,2,3} -> {4})\n";
    cout << "============================================================\n";
    cout << "  Latency  (slots) = " << result.latency << "\n";
    cout << "  Resource (tx)    = " << result.resource << "\n";
    cout << "============================================================\n";

    return 0;
}
