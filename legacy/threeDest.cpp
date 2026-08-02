#include <bits/stdc++.h>
using namespace std;

double safe(double x) { return (fabs(x) < 1e-12) ? 0.0 : x; }

// ETX for two destinations {a,b} with pairwise prob pab
double etx2(double pa, double pb, double pab) {
    double none = 1.0 - pa - pb + pab;
    double onlyA = pa - pab;
    double onlyB = pb - pab;
    double both  = pab;

    // contributions: onlyA*(1 + 1/pb), onlyB*(1 + 1/pa), both*1
    double rhs = 0.0;
    rhs += onlyA * (1.0 + 1.0 / pb);
    rhs += onlyB * (1.0 + 1.0 / pa);
    rhs += both   * 1.0;

    double etx;
    if (fabs(1.0 - none) < 1e-12) etx = 1e9; // impossible case
    else etx = (rhs + none) / (1.0 - none);
    return etx;
}

// ETX for three destinations {1,2,3}, using etx2 for 2-dest subproblems
double etx3(double p1, double p2, double p3,
            double p12, double p13, double p23, double p123) {

    double only1  = p1 - p12 - p13 + p123;
    double only2  = p2 - p12 - p23 + p123;
    double only3  = p3 - p13 - p23 + p123;
    double only12 = p12 - p123;
    double only13 = p13 - p123;
    double only23 = p23 - p123;
    double all3   = p123;
    double none   = 1.0 - (p1 + p2 + p3) + (p12 + p13 + p23) - p123;

    // numerator = none + sum_{R!=none} P(R) * (1 + rem(R))
    double rhs = none;

    // For only1 branch: remaining nodes are 2 and 3:
    rhs += only1 * (1.0 + etx2(p2, p3, p23));

    // For only2 branch:
    rhs += only2 * (1.0 + etx2(p1, p3, p13));

    // For only3 branch:
    rhs += only3 * (1.0 + etx2(p1, p2, p12));

    // For 1&2 only: remaining node is 3 -> cost = 1 + 1/p3
    rhs += only12 * (1.0 + 1.0 / p3);

    // For 1&3 only: remaining node is 2 -> cost = 1 + 1/p2
    rhs += only13 * (1.0 + 1.0 / p2);

    // For 2&3 only: remaining node is 1 -> cost = 1 + 1/p1
    rhs += only23 * (1.0 + 1.0 / p1);

    // For all3: cost = 1 (current transmission did it)
    rhs += all3 * 1.0;

    if (fabs(1.0 - none) < 1e-12) return 1e9;
    double result = rhs / (1.0 - none);
    return result;
}

int main() {
    double p1, p2, p3, p12, p13, p23, p123;

    cout << "Enter individual success probabilities p1 p2 p3 (0-1, space separated): ";
    cin >> p1 >> p2 >> p3;

    // pairwise validation
    while (true) {
        cout << "Enter p12 (between " << max(0.0, p1 + p2 - 1.0) << " and " << min(p1, p2) << "): ";
        cin >> p12;
        cout << "Enter p13 (between " << max(0.0, p1 + p3 - 1.0) << " and " << min(p1, p3) << "): ";
        cin >> p13;
        cout << "Enter p23 (between " << max(0.0, p2 + p3 - 1.0) << " and " << min(p2, p3) << "): ";
        cin >> p23;

        if (p12 >= max(0.0, p1 + p2 - 1.0) && p12 <= min(p1, p2) &&
            p13 >= max(0.0, p1 + p3 - 1.0) && p13 <= min(p1, p3) &&
            p23 >= max(0.0, p2 + p3 - 1.0) && p23 <= min(p2, p3)) break;
        cout << "Invalid pairwise probabilities! Try again.\n";
    }

    // triple validation using the stricter lower bound
    double lower = max(0.0, (2.0*(p12 + p13 + p23) - (p1 + p2 + p3))/3.0);
    double upper = min({p12, p13, p23});
    while (true) {
        cout << "Enter p123 (between " << lower << " and " << upper << "): ";
        cin >> p123;
        if (p123 >= lower && p123 <= upper) break;
        cout << "Invalid triple probability! Try again.\n";
    }

    // compute and print Venn regions
    double only1  = p1 - p12 - p13 + p123;
    double only2  = p2 - p12 - p23 + p123;
    double only3  = p3 - p13 - p23 + p123;
    double only12 = p12 - p123;
    double only13 = p13 - p123;
    double only23 = p23 - p123;
    double all3   = p123;
    double none   = 1.0 - (p1 + p2 + p3) + (p12 + p13 + p23) - p123;

    cout << fixed << setprecision(4) << "\n--- Outcome probabilities ---\n";
    cout << "000 none  : " << safe(none)  << "\n";
    cout << "001 only1 : " << safe(only1) << "\n";
    cout << "010 only2 : " << safe(only2) << "\n";
    cout << "100 only3 : " << safe(only3) << "\n";
    cout << "011 1&2   : " << safe(only12) << "\n";
    cout << "101 1&3   : " << safe(only13) << "\n";
    cout << "110 2&3   : " << safe(only23) << "\n";
    cout << "111 all3  : " << safe(all3)  << "\n";
    cout << "Sum = " << (none + only1 + only2 + only3 + only12 + only13 + only23 + all3) << "\n";

    // final ETX
    double etx = etx3(p1,p2,p3,p12,p13,p23,p123);
    cout << "\nExpected Transmission Cost (ETX for 0->{1,2,3}) = " << etx << "\n";

    return 0;
}