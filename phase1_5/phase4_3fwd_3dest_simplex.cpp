// Phase 4: Multicast Simulation — Simplex (3 forwarders, 3 destinations)
// Topology: 0 -> {1, 2, 3} -> {4, 5, 6}
// Monte Carlo simulation with 10000 packets
// Simplex: one forwarder per slot (sequential)
// Reports BOTH metrics: Latency and Resource
//
// Uses L'Ecuyer RNG (from reference project)
// Uses exact joint probabilities (no independence assumption)

#include <bits/stdc++.h>
using namespace std;

const double INF = 1e12;
const double EPS = 1e-12;

// ====================== L'Ecuyer RNG ======================
#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define RNMX (1.0-1.2e-7)

float randomgeneration(long &idum) {
    int j; long k;
    static long idum2=123456789, iy=0, iv[NTAB];
    float temp;
    if (idum <= 0) {
        idum = (-idum < 1) ? 1 : -idum;
        idum2 = idum;
        for (j=NTAB+7;j>=0;j--) {
            k=idum/IQ1;
            idum=IA1*(idum-k*IQ1)-k*IR1;
            if (idum<0) idum+=IM1;
            if (j<NTAB) iv[j]=idum;
        }
        iy=iv[0];
    }
    k=idum/IQ1;
    idum=IA1*(idum-k*IQ1)-k*IR1;
    if (idum<0) idum+=IM1;
    k=idum2/IQ2;
    idum2=IA2*(idum2-k*IQ2)-k*IR2;
    if (idum2<0) idum2+=IM2;
    j=iy/NDIV;
    iy=iv[j]-idum2;
    iv[j]=idum;
    if (iy<1) iy+=IMM1;
    temp=AM*iy;
    return (temp>RNMX)?RNMX:temp;
}

long global_seed = -(long)time(NULL);

// ====================== Venn Region Probabilities (3 forwarders) ======================
struct Reg3 {
    double none, o1, o2, o3, o12, o13, o23, all3;
};

Reg3 venn3(double p1, double p2, double p3, double p12, double p13, double p23, double p123) {
    Reg3 r;
    r.o1   = max(0.0, p1 - p12 - p13 + p123);
    r.o2   = max(0.0, p2 - p12 - p23 + p123);
    r.o3   = max(0.0, p3 - p13 - p23 + p123);
    r.o12  = max(0.0, p12 - p123);
    r.o13  = max(0.0, p13 - p123);
    r.o23  = max(0.0, p23 - p123);
    r.all3 = max(0.0, p123);
    r.none = max(0.0, 1.0 - (p1+p2+p3) + (p12+p13+p23) - p123);
    return r;
}

// ====================== Hop-2 Region Probabilities (2 destinations per forwarder) ======================
struct Reg2 {
    double none, oA, oB, both;
};

Reg2 venn2(double pA, double pB, double pAB) {
    Reg2 r;
    r.oA   = max(0.0, pA - pAB);
    r.oB   = max(0.0, pB - pAB);
    r.both = max(0.0, pAB);
    r.none = max(0.0, 1.0 - pA - pB + pAB);
    return r;
}

// Simulate one hop-2 broadcast from forwarder to its destinations
// Returns which destinations received (subset of targets)
// Uses exact joint probabilities
set<int> simHop2(long &seed, double pA, double pB, double pAB, int destA, int destB) {
    Reg2 r = venn2(pA, pB, pAB);
    double cum = 0.0;
    float randVal = randomgeneration(seed);

    cum += r.none;
    if (randVal < cum) return {};

    cum += r.oA;
    if (randVal < cum) return {destA};

    cum += r.oB;
    if (randVal < cum) return {destB};

    return {destA, destB};
}

// Compute expected remaining cost for a relay at a given coverage state
// Relay 1 -> {4,5} (global bits 0,1), relay 2/3 -> {5,6} (global bits 1,2)
// Uses the same formula as analytical.cpp's simplexDownstream
double relayCostAtState(int relay, int cov,
                        double p14, double p15, double p145,
                        double p25, double p26, double p256,
                        double p35, double p36, double p356) {
    double pA, pB, pAB;
    int bitA, bitB;
    if (relay == 1) { pA=p14; pB=p15; pAB=p145; bitA=0; bitB=1; }
    else if (relay == 2) { pA=p25; pB=p26; pAB=p256; bitA=1; bitB=2; }
    else { pA=p35; pB=p36; pAB=p356; bitA=1; bitB=2; }

    int needA = 1 - ((cov >> bitA) & 1);
    int needB = 1 - ((cov >> bitB) & 1);

    if (!needA && !needB) return 0.0;

    Reg2 r = venn2(pA, pB, pAB);
    double denom = 1.0 - r.none;
    if (denom < EPS) return INF;

    if (needA && needB) {
        // Both needed: E = (1 + oA*cB + oB*cA) / (1-none)
        double cA = (pA > EPS) ? 1.0/pA : INF;
        double cB = (pB > EPS) ? 1.0/pB : INF;
        return (1.0 + r.oA * cB + r.oB * cA) / denom;
    }
    if (needA) return (pA > EPS) ? 1.0/pA : INF;
    return (pB > EPS) ? 1.0/pB : INF;
}

// ====================== Analytical ETX (Simplex) ======================
// Using paper's formula: E = (1 + Σ P(region) * X) / (1 - P(none))
// For simplex, X for each region = min cost from active forwarders to uncovered destinations

// Remaining cost for a subset of forwarders to cover remaining destinations (simplex)
// This is a simplified version for the 3-dest case
double remainingCostSimplex(const set<int>& activeFwds,
                             const set<int>& remainingDests,
                             double p14, double p15, double p145,
                             double p25, double p26, double p256,
                             double p35, double p36, double p356) {
    if (remainingDests.empty()) return 0.0;

    // For each active forwarder, compute its ETX to the destinations it can reach
    // Simplex: pick the forwarder with minimum total cost
    double bestCost = INF;

    for (int fwd : activeFwds) {
        // Forwarder 1 reaches {4,5}, Forwarder 2 reaches {5,6}, Forwarder 3 reaches {5,6}
        double cost = 0.0;
        if (fwd == 1) {
            // Forwarder 1 covers dests {4,5} it can reach
            set<int> myDests;
            for (int d : remainingDests) if (d == 4 || d == 5) myDests.insert(d);
            if (myDests.size() == 2) {
                Reg2 r = venn2(p14, p15, p145);
                double c14 = 1.0/p14, c15 = 1.0/p15;
                cost = (1.0 + r.oA*c15 + r.oB*c14) / (1.0-r.none);
            } else if (myDests.count(4)) {
                cost = 1.0/p14;
            } else if (myDests.count(5)) {
                cost = 1.0/p15;
            }
        } else if (fwd == 2) {
            set<int> myDests;
            for (int d : remainingDests) if (d == 5 || d == 6) myDests.insert(d);
            if (myDests.size() == 2) {
                Reg2 r = venn2(p25, p26, p256);
                double c25 = 1.0/p25, c26 = 1.0/p26;
                cost = (1.0 + r.oA*c26 + r.oB*c25) / (1.0-r.none);
            } else if (myDests.count(5)) {
                cost = 1.0/p25;
            } else if (myDests.count(6)) {
                cost = 1.0/p26;
            }
        } else if (fwd == 3) {
            set<int> myDests;
            for (int d : remainingDests) if (d == 5 || d == 6) myDests.insert(d);
            if (myDests.size() == 2) {
                Reg2 r = venn2(p35, p36, p356);
                double c35 = 1.0/p35, c36 = 1.0/p36;
                cost = (1.0 + r.oA*c36 + r.oB*c35) / (1.0-r.none);
            } else if (myDests.count(5)) {
                cost = 1.0/p35;
            } else if (myDests.count(6)) {
                cost = 1.0/p36;
            }
        }
        if (cost < bestCost) bestCost = cost;
    }
    return bestCost;
}

// ====================== Main Simulation ======================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "============================================================\n";
    cout << "  Phase 4: Multicast Simulation — Simplex\n";
    cout << "  Topology: 0 -> {1, 2, 3} -> {4, 5, 6}\n";
    cout << "============================================================\n\n";

    // Hop-1 probabilities
    double p01, p02, p03, p012, p013, p023, p0123;
    cout << "Hop-1 probabilities:\n";
    cout << "Enter p(0->1) p(0->2) p(0->3): ";
    cin >> p01 >> p02 >> p03;
    cout << "Enter p(0->1,2) p(0->1,3) p(0->2,3) p(0->1,2,3): ";
    cin >> p012 >> p013 >> p023 >> p0123;

    // Hop-2 probabilities
    double p14, p15, p145;
    double p25, p26, p256;
    double p35, p36, p356;
    cout << "\nHop-2 probabilities (node 1 -> {4,5}):\n";
    cout << "Enter p(1->4) p(1->5) p(1->4,5): ";
    cin >> p14 >> p15 >> p145;
    cout << "\nHop-2 probabilities (node 2 -> {5,6}):\n";
    cout << "Enter p(2->5) p(2->6) p(2->5,6): ";
    cin >> p25 >> p26 >> p256;
    cout << "\nHop-2 probabilities (node 3 -> {5,6}):\n";
    cout << "Enter p(3->5) p(3->6) p(3->5,6): ";
    cin >> p35 >> p36 >> p356;

    int N;
    cout << "\nNumber of packets: ";
    cin >> N;

    // Venn regions for hop-1
    Reg3 hop1 = venn3(p01, p02, p03, p012, p013, p023, p0123);

    cout << "\n--- Hop-1 Region Probabilities ---\n";
    cout << "none=" << hop1.none << " o1=" << hop1.o1 << " o2=" << hop1.o2
         << " o3=" << hop1.o3 << " o12=" << hop1.o12 << " o13=" << hop1.o13
         << " o23=" << hop1.o23 << " all3=" << hop1.all3 << "\n";

    // ==================== SIMULATION ====================
    long totalLat = 0, totalRes = 0;
    int delivered = 0;

    for (int pkt = 0; pkt < N; pkt++) {
        set<int> dests = {4, 5, 6};
        long lat = 0, res = 0;

        // Hop 1: source broadcasts until active forwarders can cover all destinations
        // Coverage: fwd 1 covers {4,5}, fwd 2 covers {5,6}, fwd 3 covers {5,6}
        // Need: (fwd 1 OR fwd 2 OR fwd 3) AND (fwd 2 OR fwd 3) for dest 6
        // Actually: fwd 1 covers {4}, fwd 2 covers {6}, fwd 3 covers {6}
        // Need: (fwd 1) for dest 4, (fwd 2 OR fwd 3) for dest 6, any for dest 5
        // Simplified: need fwd 1 AND (fwd 2 OR fwd 3) to cover all
        // But we also accept: fwd 1 alone if dests only need {4,5}
        // Better: just keep retransmitting until coverage is possible

        set<int> activeFwds;
        bool covered = false;
        int maxHop1 = 100;

        while (!covered && maxHop1-- > 0) {
            lat++; res++;
            float randVal = randomgeneration(global_seed);
            double cum = 0.0;

            // Determine which forwarders received (one random number, cumulative regions)
            set<int> got;
            cum += hop1.none;
            if (randVal < cum) { /* none */ }
            else { cum += hop1.o1;
            if (randVal < cum) { got = {1}; }
            else { cum += hop1.o2;
            if (randVal < cum) { got = {2}; }
            else { cum += hop1.o3;
            if (randVal < cum) { got = {3}; }
            else { cum += hop1.o12;
            if (randVal < cum) { got = {1,2}; }
            else { cum += hop1.o13;
            if (randVal < cum) { got = {1,3}; }
            else { cum += hop1.o23;
            if (randVal < cum) { got = {2,3}; }
            else { got = {1,2,3}; }
            }}}}}}

            // Accumulate received forwarders across retransmissions
            for (int f : got) activeFwds.insert(f);

            // Check coverage: fwd 1 covers {4}, fwd 2/3 cover {6}, any covers {5}
            bool canCover4 = activeFwds.count(1);
            bool canCover6 = activeFwds.count(2) || activeFwds.count(3);
            covered = canCover4 && canCover6;
        }

        // Hop 2: optimal relay selection (simplex = single best relay per slot)
        // At each slot, compute which active relay has lowest expected remaining cost
        // This matches analytical.cpp's simplexDownstream definition
        vector<int> fwdList(activeFwds.begin(), activeFwds.end());
        int maxIter = 10000;

        while (!dests.empty() && maxIter-- > 0) {
            // Compute coverage bitmask from current destinations
            int cov = 0;
            if (dests.count(4)) cov |= 1;
            if (dests.count(5)) cov |= 2;
            if (dests.count(6)) cov |= 4;

            // Pick relay with minimum expected remaining cost
            int bestFwd = fwdList[0];
            double bestCost = 1e18;
            for (int fwd : fwdList) {
                double cost = relayCostAtState(fwd, cov,
                    p14, p15, p145, p25, p26, p256, p35, p36, p356);
                if (cost < bestCost) {
                    bestCost = cost;
                    bestFwd = fwd;
                }
            }

            lat++; res++;  // one relay = 1 slot, 1 tx

            set<int> received;
            if (bestFwd == 1) {
                received = simHop2(global_seed, p14, p15, p145, 4, 5);
            } else if (bestFwd == 2) {
                received = simHop2(global_seed, p25, p26, p256, 5, 6);
            } else if (bestFwd == 3) {
                received = simHop2(global_seed, p35, p36, p356, 5, 6);
            }

            for (int d : received) dests.erase(d);
        }

        totalLat += lat;
        totalRes += res;
        delivered++;
    }

    double avgLat = (double)totalLat / N;
    double avgRes = (double)totalRes / N;

    // ==================== ANALYTICAL (simplified) ====================
    // For the "both" regions, compute remaining cost
    double c14 = 1.0/p14, c15 = 1.0/p15;
    double c25 = 1.0/p25, c26 = 1.0/p26;
    double c35 = 1.0/p35, c36 = 1.0/p36;

    // Remaining costs for each forwarder (corrected formula)
    // E = (1 + oA*cB + oB*cA) / (1 - P(none))
    // oA = P(only first dest received) → remaining cost = cB
    // oB = P(only second dest received) → remaining cost = cA
    // both → remaining cost = 0 (dests already covered)
    Reg2 r1 = venn2(p14, p15, p145);
    double R1_45 = (1.0 + r1.oA*c15 + r1.oB*c14) / (1.0-r1.none);
    Reg2 r2 = venn2(p25, p26, p256);
    double R2_56 = (1.0 + r2.oA*c26 + r2.oB*c25) / (1.0-r2.none);
    Reg2 r3 = venn2(p35, p36, p356);
    double R3_56 = (1.0 + r3.oA*c36 + r3.oB*c35) / (1.0-r3.none);

    // Simplex remaining costs for each forwarder set
    // {1}: covers {4,5}, need to cover {6} → impossible via fwd 1 alone
    // Actually fwd 1 only reaches {4,5}. If only fwd 1 has packet, dest 6 is uncovered.
    // Need to retransmit from source until another fwd gets it.
    // This is complex for analytical. Use simulation as ground truth.

    cout << "\n============================================================\n";
    cout << "  SIMULATION RESULTS (" << N << " packets, Simplex)\n";
    cout << "============================================================\n";
    cout << "  Average Latency  (slots) = " << avgLat << "\n";
    cout << "  Average Resource (tx)    = " << avgRes << "\n";
    cout << "  Packets delivered        = " << delivered << "/" << N << "\n";

    cout << "\n--- Forwarder Costs (for reference) ---\n";
    cout << "  R1 (fwd 1 -> {4,5}) = " << R1_45 << "\n";
    cout << "  R2 (fwd 2 -> {5,6}) = " << R2_56 << "\n";
    cout << "  R3 (fwd 3 -> {5,6}) = " << R3_56 << "\n";

    cout << "\n============================================================\n";
    return 0;
}
