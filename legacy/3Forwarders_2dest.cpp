#include <bits/stdc++.h>
using namespace std;

const double INF = 1e18;
double EPS = 1e-12;

// return 0 for tiny negative values, otherwise x
double safe(double x) { return (x < EPS ? 0.0 : x); }

// safe min that handles INF
double min_inf(double a, double b) {
    if (!isfinite(a)) return b;
    if (!isfinite(b)) return a;
    return min(a, b);
}
double min_inf(initializer_list<double> L) {
    double best = INF;
    for (double v : L) if (isfinite(v)) best = min(best, v);
    return best;
}

// safe division: if denom <= 0 -> INF
double safe_divide(double num, double den) {
    if (!(den > EPS)) return INF;
    return num / den;
}

// ETX formula for node 2 -> {4,5} given p24,p25,p245 and c24,c25
double etx_2_to_45(double p24, double p25, double p245, double c24, double c25) {
    double den = (p24 + p25 - p245);
    if (!(den > EPS)) return INF;
    double num = (p24 - p245) * (1 + c25) + (p25 - p245) * (1 + c24) + p245 + (1 - p24 - p25 + p245);
    return safe_divide(num, den);
}

// compute ETX when two receivers (i and j) are downstream from the same transmitter
// formula pattern used in original code, but guarded
double etx_two_receivers(double pi, double pj, double pij, double ci, double cj) {
    double den = (pi + pj - pij);
    if (!(den > EPS)) return INF;
    double term_i = (pi - pij) * (1 + cj);
    double term_j = (pj - pij) * (1 + ci);
    double term_both = pij * (1 + min_inf(ci, cj));
    double term_none = (1 - pi - pj + pij);
    double num = term_i + term_j + term_both + term_none;
    return safe_divide(num, den);
}

int main() {
    cout << "=== COST CALCULATION (DEPENDENT CASE) ===\n\n";

    double p01, p02, p03, p12, p13, p23, p123;
    cout << "Enter p01 p02 p03  (prob of success 0->1, 0->2, 0->3): ";
    cin >> p01 >> p02 >> p03;

    cout << "Enter joint probs p12 p13 p23 p123 (enter -1 -1 -1 -1 for independence): ";
    cin >> p12 >> p13 >> p23 >> p123;
    if (p12 < 0) {
        p12  = p01 * p02;
        p13  = p01 * p03;
        p23  = p02 * p03;
        p123 = p01 * p02 * p03;
    }

    // ===== ASK USER ABOUT LINKS =====
    double p14=-1, p15=-1, p24=-1, p25=-1, p34=-1, p35=-1;
    char ans;
    cout << "\nEnter link existence and probabilities (y/n / value when y):\n";

    cout << "Is there a link 1->4? (y/n): "; cin >> ans;
    if (ans=='y'||ans=='Y'){ cout << "Enter p14: "; cin >> p14; }

    cout << "Is there a link 1->5? (y/n): "; cin >> ans;
    if (ans=='y'||ans=='Y'){ cout << "Enter p15: "; cin >> p15; }

    cout << "Is there a link 2->4? (y/n): "; cin >> ans;
    if (ans=='y'||ans=='Y'){ cout << "Enter p24: "; cin >> p24; }
    cout << "Is there a link 2->5? (y/n): "; cin >> ans;
    if (ans=='y'||ans=='Y'){ cout << "Enter p25: "; cin >> p25; }

    cout << "Is there a link 3->4? (y/n): "; cin >> ans;
    if (ans=='y'||ans=='Y'){ cout << "Enter p34: "; cin >> p34; }
    cout << "Is there a link 3->5? (y/n): "; cin >> ans;
    if (ans=='y'||ans=='Y'){ cout << "Enter p35: "; cin >> p35; }

    double p245;
    cout << "Enter joint probability p245 (or -1 for independence): ";
    cin >> p245;
    if (p245 < 0) {
        if (p24 > 0 && p25 > 0) p245 = p24 * p25;
        else p245 = 0.0;
    }

    // ===== COSTS =====
    auto cost = [&](double p){ return (p > EPS ? 1.0 / p : INF); };

    double c01 = cost(p01);
    double c02 = cost(p02);
    double c03 = cost(p03);

    double c14 = cost(p14);
    double c15 = cost(p15);
    double c24 = cost(p24);
    double c25 = cost(p25);
    double c34 = cost(p34);
    double c35 = cost(p35);

    double c245 = etx_2_to_45((p24>0?p24:0.0), (p25>0?p25:0.0), p245, c24, c25);

    // c05 is ETX from node 0 to destination 5 via receivers 2 and 3 (as in original code)
    // guarded: require denominator > 0
    double c05 = etx_two_receivers(p02, p03, p23, c25, c35);
    double c04 = etx_two_receivers(p01, p02, p12, c14, c24);

    // --- Region probabilities ---
    double only1  = safe(p01 - p12 - p13 + p123);
    double only2  = safe(p02 - p12 - p23 + p123);
    double only3  = safe(p03 - p13 - p23 + p123);
    double only12 = safe(p12 - p123);
    double only13 = safe(p13 - p123);
    double only23 = safe(p23 - p123);
    double all3   = safe(p123);
    double none   = safe(1 - (p01 + p02 + p03) + (p12 + p13 + p23) - p123);

    // normalize tiny rounding errors so sum of partition probabilities <= 1
    double sumParts = none + only1 + only2 + only3 + only12 + only13 + only23 + all3;
    if (sumParts > 1.0 + 1e-9) {
        // scale down proportionally if numerical drift occurred
        double factor = 1.0 / sumParts;
        none *= factor; only1 *= factor; only2 *= factor; only3 *= factor;
        only12 *= factor; only13 *= factor; only23 *= factor; all3 *= factor;
    }

    cout << "\n--- Region Probabilities ---\n";
    cout << fixed << setprecision(6);
    cout << "none=" << none << ", only1=" << only1 << ", only2=" << only2
         << ", only3=" << only3 << ", only12=" << only12 << ", only13=" << only13
         << ", only23=" << only23 << ", all3=" << all3 << "\n\n";

    // --- Region costs R(S) ---
    // R1 : forwarder set {1} cost to reach {4,5} (via node1 -> maybe to 4 or 5)
    // If some required downstream link missing, cost becomes INF
    double R1 = INF, R2 = INF, R3 = INF;
    // R1: node1 needs to reach D1 (4) and D2 (5). Using previous logic: c14 + c05 (node1->4 direct + 0->5 cost)
    // But original code used c14 + c05 (interpreting some path). To keep behavior, set R1 = c14 + c05 if finite
    if (isfinite(c14) && isfinite(c05)) R1 = c14 + c05;

    // R2: forwarder set {2} : use c245 (computed)
    if (isfinite(c245)) R2 = c245;

    // R3: forwarder set {3}: c04 + c35
    if (isfinite(c04) && isfinite(c35)) R3 = c04 + c35;

    // Two-element forwarder-sets
    double R12 = INF, R13 = INF, R23 = INF, R123 = INF;
    // R12: {1,2} - try c14 + c25 or R2 whichever is better (but only finite values)
    double cand_R12_1 = isfinite(c14) && isfinite(c25) ? (c14 + c25) : INF;
    R12 = min_inf({cand_R12_1, R2});

    // R13: min(c14+c35, c04 + c05)
    double cand_R13_1 = isfinite(c14) && isfinite(c35) ? (c14 + c35) : INF;
    double cand_R13_2 = isfinite(c04) && isfinite(c05) ? (c04 + c05) : INF;
    R13 = min_inf({cand_R13_1, cand_R13_2});

    // R23: min(c24+c35, R2)
    double cand_R23_1 = isfinite(c24) && isfinite(c35) ? (c24 + c35) : INF;
    R23 = min_inf({cand_R23_1, R2});

    // R123: minimum among listed options
    double opt1 = (isfinite(c14) && isfinite(c25)) ? (c14 + c25) : INF;
    double opt2 = (isfinite(c14) && isfinite(c35)) ? (c14 + c35) : INF;
    double opt3 = (isfinite(c24) && isfinite(c35)) ? (c24 + c35) : INF;
    R123 = min_inf({opt1, opt2, opt3, R2});

    cout << "--- Region Costs ---\n";
    auto showVal = [&](double v){ if (!isfinite(v)) cout << "INF"; else cout << fixed << setprecision(6) << v; };
    cout << "{1}="; showVal(R1); cout << ", {2}="; showVal(R2);
    cout << ", {3}="; showVal(R3);
    cout << ", {1,2}="; showVal(R12);
    cout << ", {1,3}="; showVal(R13);
    cout << ", {2,3}="; showVal(R23);
    cout << ", {1,2,3}="; showVal(R123);
    cout << "\n\n";

    // numerator as per original structure: sum over regionProb * (1 + R_region) + none * 1
    double numerator = 0.0;
    auto add_region = [&](double prob, double Rregion) {
        if (prob <= 0) return;
        if (!isfinite(Rregion)) {
            // if Rregion is INF, those outcomes are infeasible -> they contribute prob * (1 + INF) which is INF
            // but logically if a forwarder set cannot reach DS then that region should not contribute to successful deliveries.
            // For safety, treat as skip (since in original numerator they multiplied only for finite costs).
            return;
        }
        numerator += prob * (1.0 + Rregion);
    };

    add_region(only1, R1);
    add_region(only2, R2);
    add_region(only3, R3);
    add_region(only12, R12);
    add_region(only13, R13);
    add_region(only23, R23);
    add_region(all3, R123);

    // The 'none' region means no forwarder received the packet in that transmission attempt.
    // In the original code they added none once (as + none). We'll keep same semantics.
    numerator += none;

    cout << "numerator = " << numerator << "\n";
    double denom = 1.0 - none;
    double Etx;
    if (!(denom > EPS)) {
        Etx = INF;
    } else {
        Etx = numerator / denom;
    }

    cout << "\n--- Link and intermediate costs ---\n";
    cout << "c04 = "; showVal(c04); cout << "\n";
    cout << "c05 = "; showVal(c05); cout << "\n";
    cout << "c14 = "; showVal(c14); cout << "\n";
    cout << "c15 = "; showVal(c15); cout << "\n";
    cout << "c24 = "; showVal(c24); cout << "\n";
    cout << "c25 = "; showVal(c25); cout << "\n";
    cout << "c34 = "; showVal(c34); cout << "\n";
    cout << "c35 = "; showVal(c35); cout << "\n";
    cout << "c245 = "; showVal(c245); cout << "\n\n";

    cout << "ETX (0 -> {4,5}) = ";
    if (!isfinite(Etx)) cout << "INF (denominator zero or no reception possible)\n";
    else cout << fixed << setprecision(6) << Etx << "\n";

    // Find best forwarder set among finite region-costs
    vector<pair<string,double>> setCosts = {
        {"{1}", R1}, {"{2}", R2}, {"{3}", R3},
        {"{1,2}", R12}, {"{1,3}", R13}, {"{2,3}", R23}, {"{1,2,3}", R123}
    };
    double bestVal = INF;
    for (auto &p : setCosts) if (isfinite(p.second)) bestVal = min(bestVal, p.second);

    if (!isfinite(bestVal)) {
        cout << "No feasible forwarder set (all costs INF)\n";
    } else {
        cout << "Best forwarder set(s) with cost = " << fixed << setprecision(6) << bestVal << ": ";
        bool first = true;
        for (auto &p : setCosts) {
            if (isfinite(p.second) && fabs(p.second - bestVal) < 1e-8) {
                if (!first) cout << ", ";
                cout << p.first;
                first = false;
            }
        }
        cout << "\n";
    }

    return 0;
}