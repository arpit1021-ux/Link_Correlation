#include <bits/stdc++.h>
using namespace std;

static constexpr double INF = 1e12;
static constexpr double EPS  = 1e-12;

double safe_zero(double x) { return (fabs(x) < EPS) ? 0.0 : x; }

bool pairwise_valid(double a, double b, double ab) {
    double low = max(0.0, a + b - 1.0);
    double high = min(a, b);
    return (ab + EPS >= low && ab <= high + EPS);
}

bool triple_valid_lower(double p1, double p2, double p3,
                        double p12, double p13, double p23, double p123) {
    double lb = max(0.0, p1 + p2 + p3 - (p12 + p13 + p23));
    return (p123 + EPS >= lb);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(&cout);

    cout << "Enter reception marginals p0-1 p0-2 p0-3 (space separated, 0..1):" << endl;
    double p1, p2, p3;
    if (!(cin >> p1 >> p2 >> p3)) { cerr << "Input error\n"; return 1; }

    cout << "Enter reception pairwise joints p0-(1-2) p0-(1-3) p0-(2-3) (or -1 -1 -1 to assume independence):" << endl;
    double p12, p13, p23;
    cin >> p12 >> p13 >> p23;
    if (p12 < 0 || p13 < 0 || p23 < 0) {
        p12 = p1*p2; p13 = p1*p3; p23 = p2*p3;
        cout << "(Assuming independence for reception pairs)" << endl;
    }

    cout << "Enter reception triple joint p0-(1-2-3) (or -1 to assume independence):" << endl;
    double p123; cin >> p123;
    if (p123 < 0) { p123 = p1*p2*p3; cout << "(Assuming independence for triple reception)" << endl; }

    if (!pairwise_valid(p1,p2,p12) || !pairwise_valid(p1,p3,p13) || !pairwise_valid(p2,p3,p23))
        cerr << "Warning: pairwise inconsistent.\n";
    if (!triple_valid_lower(p1,p2,p3,p12,p13,p23,p123))
        cerr << "Warning: triple lower bound violated.\n";

    cout << "Enter forward marginals p1-4 p2-4 p3-4 (space separated, 0..1):" << endl;
    double p14, p24, p34;
    cin >> p14 >> p24 >> p34;

    // region probabilities
    double only1  = safe_zero(p1 - p12 - p13 + p123);
    double only2  = safe_zero(p2 - p12 - p23 + p123);
    double only3  = safe_zero(p3 - p13 - p23 + p123);
    double only12 = safe_zero(p12 - p123);
    double only13 = safe_zero(p13 - p123);
    double only23 = safe_zero(p23 - p123);
    double all3   = safe_zero(p123);
    double none   = safe_zero(1 - (p1+p2+p3) + (p12+p13+p23) - p123);

    cout << fixed << setprecision(6);
    cout << "\n--- reception-region probabilities ---\n";
    cout << "P_none="<<none<<", only1="<<only1<<", only2="<<only2<<", only3="<<only3
         <<", only12="<<only12<<", only13="<<only13<<", only23="<<only23<<", all3="<<all3<<"\n\n";

    auto rem_single = [&](double pf){ return (pf <= 0.0 ? INF : 1.0/pf); };
    auto rem_pair   = [&](double a,double b){ double best=max(a,b); return (best<=0.0?INF:1.0/best); };
    auto rem_triple = [&](double a,double b,double c){ double best=max({a,b,c}); return (best<=0.0?INF:1.0/best); };

    double R_only1=rem_single(p14), R_only2=rem_single(p24), R_only3=rem_single(p34);
    double R_only12=rem_pair(p14,p24), R_only13=rem_pair(p14,p34), R_only23=rem_pair(p24,p34);
    double R_all3=rem_triple(p14,p24,p34);

    struct Region {
        string name;
        double P;
        double R;
        double contrib;
    };

    vector<Region> regions = {
        {"{1}",  only1,  R_only1,  only1*(1+R_only1)},
        {"{2}",  only2,  R_only2,  only2*(1+R_only2)},
        {"{3}",  only3,  R_only3,  only3*(1+R_only3)},
        {"{1,2}",only12, R_only12, only12*(1+R_only12)},
        {"{1,3}",only13, R_only13, only13*(1+R_only13)},
        {"{2,3}",only23, R_only23, only23*(1+R_only23)},
        {"{1,2,3}",all3, R_all3,   all3*(1+R_all3)}
    };

    double numerator = none;
    for (auto &r: regions) numerator += r.contrib;
    double denom = 1 - none;
    double ETX = (denom <= EPS ? INF : numerator/denom);

    cout << "--- Remaining Costs R(S) ---\n";
    for (auto &r: regions) {
        cout << "R(" << r.name << ") = " << (r.R>=INF/2?INFINITY:r.R) << "\n";
    }

    cout << "\n--- Contributions P_S*(1+R(S)) ---\n";
    for (auto &r: regions) {
        cout << r.name << " contrib = " << r.contrib << "\n";
    }

    cout << "numerator = " << numerator << ", denominator = " << denom << "\n";

    // 🟩 NEW PART: Find best forwarder set with minimum total (1+R(S))
    double bestCost = INF;
    for (auto &r : regions) {
        if (r.P > 0 && (1 + r.R) < bestCost) bestCost = (1 + r.R);
    }

    vector<string> bestSets;
    for (auto &r : regions) {
        if (fabs((1 + r.R) - bestCost) < EPS && r.P > 0)
            bestSets.push_back(r.name);
    }

    cout << "\n--- Final ETX ---\n";
    if (ETX >= INF/2)
        cout << "ETX = ∞ (delivery impossible)\n";
    else
        cout << "ETX = " << ETX << "\n";

    cout << "Best forwarder set(s) giving minimum (1 + R(S)) = " << bestCost << "\n";
    cout << "Forwarder set(s): ";
    for (auto &s : bestSets) cout << s << " ";
    cout << "\n";

    return 0;
}
