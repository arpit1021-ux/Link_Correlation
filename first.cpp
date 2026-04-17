#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

double probability(int success, int total) {
    if (total == 0) return 0.0;
    return (double)success / total;
}

int main() {
    int totalPackets;
    cout << "Enter total packets sent: ";
    cin >> totalPackets;

    int link01, link02;
    cout << "Is there a link from 0->1? (1=Yes, 0=No): ";
    cin >> link01;
    cout << "Is there a link from 0->2? (1=Yes, 0=No): ";
    cin >> link02;

    vector<int> packets01(totalPackets, 0), packets02(totalPackets, 0);
    int success01 = 0, success02 = 0, common = 0;

    if (link01) {
        cout << "Enter outcomes for 0->1 (1=success, 0=failure):\n";
        for (int i = 0; i < totalPackets; i++) {
            cin >> packets01[i];
            if (packets01[i] == 1) success01++;
        }
    }

    if (link02) {
        cout << "Enter outcomes for 0->2 (1=success, 0=failure):\n";
        for (int i = 0; i < totalPackets; i++) {
            cin >> packets02[i];
            if (packets02[i] == 1) success02++;
        }
    }

    double p1 = probability(success01, totalPackets);
    double p2 = probability(success02, totalPackets);

    if (link01 && link02) {
        for (int i = 0; i < totalPackets; i++) {
            if (packets01[i] == 1 && packets02[i] == 1) common++;
        }
    }

    double p12 = probability(common, totalPackets);

    double cost1 = (p1 > 0) ? 1.0 / p1 : 1e9;
    double cost2 = (p2 > 0) ? 1.0 / p2 : 1e9;

    double expected = 0.0;
    if (link01 && link02) {
        double q = p1 + p2 - p12;
        if (q > 0 && p1 > 0 && p2 > 0) {
            double only1 = p1 - p12;
            double only2 = p2 - p12;
            double extra1 = 1.0 / p2;
            double extra2 = 1.0 / p1;
            expected = (1 + only1 * extra1 + only2 * extra2) / q;
        } else {
            expected = 1e9;
        }
    }

    cout << fixed << setprecision(4);
    if (link01) cout << "\nP(0->1) = " << p1 << " , Cost = " << cost1;
    if (link02) cout << "\nP(0->2) = " << p2 << " , Cost = " << cost2;
    if (link01 && link02) {
        cout << "\nP(common) = " << p12;
        cout << "\nExpected Cost (0->{1,2}) = " << expected << endl;
    }

    return 0;
}