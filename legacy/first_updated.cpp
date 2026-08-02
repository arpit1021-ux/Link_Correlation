#include <bits/stdc++.h>
using namespace std;

int main() {
    cout << fixed << setprecision(4);

    double p01, p02, p12;
    cout << "Enter success probability p(0->1): ";
    cin >> p01;
    cout << "Enter success probability p(0->2): ";
    cin >> p02;
    cout << "Enter joint success probability p(0->1 and 0->2): ";
    cin >> p12;

    // Individual ETX
    double etx1 = (p01 > 0) ? 1.0 / p01 : 1e9;
    double etx2 = (p02 > 0) ? 1.0 / p02 : 1e9;

    // q = prob that at least one succeeds in a transmission
    double q = p01 + p02 - p12;

    double expected;
    if (q > 0 && p01 > 0 && p02 > 0) {
        double only1 = p01 - p12; // success at 1 only
        double only2 = p02 - p12; // success at 2 only

        // closed form derived from recurrence (includes the p12*1 term)
        expected = (1.0 + only1 * (1.0 / p02) + only2 * (1.0 / p01)) / q;
    } else {
        expected = 1e9; // degenerate / invalid probabilities
    }

    cout << "\n=== RESULTS ===\n";
    cout << "p(0->1) = " << p01 << " , ETX(0->1) = " << etx1 << "\n";
    cout << "p(0->2) = " << p02 << " , ETX(0->2) = " << etx2 << "\n";
    cout << "p(0->1 and 0->2) = " << p12 << "\n";
    cout << "Expected total transmissions to get packet to BOTH nodes = " << expected << "\n";

    return 0;
}