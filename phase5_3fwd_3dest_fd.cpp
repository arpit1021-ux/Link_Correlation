// Phase 5: Multicast Simulation — Full Duplex (3 forwarders, 3 destinations)
// Topology: 0 -> {1, 2, 3} -> {4, 5, 6}
// Monte Carlo simulation with 10000 packets
// Full Duplex: ALL active forwarders transmit simultaneously per time slot
// Reports BOTH metrics: Latency and Resource
// Shows comparison with Simplex baseline

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
        double cA = (pA > EPS) ? 1.0/pA : INF;
        double cB = (pB > EPS) ? 1.0/pB : INF;
        return (1.0 + r.oA * cB + r.oB * cA) / denom;
    }
    if (needA) return (pA > EPS) ? 1.0/pA : INF;
    return (pB > EPS) ? 1.0/pB : INF;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "============================================================\n";
    cout << "  Phase 5: Multicast Simulation — Full Duplex\n";
    cout << "  Topology: 0 -> {1, 2, 3} -> {4, 5, 6}\n";
    cout << "============================================================\n\n";

    double p01, p02, p03, p012, p013, p023, p0123;
    cout << "Hop-1 probabilities:\n";
    cout << "Enter p(0->1) p(0->2) p(0->3): ";
    cin >> p01 >> p02 >> p03;
    cout << "Enter p(0->1,2) p(0->1,3) p(0->2,3) p(0->1,2,3): ";
    cin >> p012 >> p013 >> p023 >> p0123;

    double p14, p15, p145, p25, p26, p256, p35, p36, p356;
    cout << "\nHop-2 (node 1 -> {4,5}): ";
    cin >> p14 >> p15 >> p145;
    cout << "Hop-2 (node 2 -> {5,6}): ";
    cin >> p25 >> p26 >> p256;
    cout << "Hop-2 (node 3 -> {5,6}): ";
    cin >> p35 >> p36 >> p356;

    int N;
    cout << "\nNumber of packets: ";
    cin >> N;

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
        set<int> activeFwds;
        bool covered = false;
        int maxHop1 = 100;

        while (!covered && maxHop1-- > 0) {
            lat++; res++;
            float randVal = randomgeneration(global_seed);
            double cum = 0.0;

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

            for (int f : got) activeFwds.insert(f);
            bool canCover4 = activeFwds.count(1);
            bool canCover6 = activeFwds.count(2) || activeFwds.count(3);
            covered = canCover4 && canCover6;
        }

        // Hop 2: FULL DUPLEX — all active forwarders transmit simultaneously
        // Relay set is recomputed each round based on current coverage
        int maxHop2 = 1000;

        while (!dests.empty() && maxHop2-- > 0) {
            lat++;  // 1 time slot

            // Determine which relays are still active (have uncovered destinations)
            vector<int> curActive;
            for (int fwd : activeFwds) {
                bool hasUncovered = false;
                if (fwd == 1 && (dests.count(4) || dests.count(5))) hasUncovered = true;
                if (fwd == 2 && (dests.count(5) || dests.count(6))) hasUncovered = true;
                if (fwd == 3 && (dests.count(5) || dests.count(6))) hasUncovered = true;
                if (hasUncovered) curActive.push_back(fwd);
            }

            if (curActive.empty()) break;  // should not happen

            res += curActive.size();  // N active relays transmit = N transmissions

            // Each active forwarder broadcasts simultaneously (independent random draws)
            set<int> allReceived;
            for (int fwd : curActive) {
                set<int> received;
                if (fwd == 1) {
                    received = simHop2(global_seed, p14, p15, p145, 4, 5);
                } else if (fwd == 2) {
                    received = simHop2(global_seed, p25, p26, p256, 5, 6);
                } else if (fwd == 3) {
                    received = simHop2(global_seed, p35, p36, p356, 5, 6);
                }
                for (int d : received) allReceived.insert(d);
            }

            // Remove delivered destinations
            for (int d : allReceived) dests.erase(d);
        }

        totalLat += lat;
        totalRes += res;
        delivered++;
    }

    double avgLat = (double)totalLat / N;
    double avgRes = (double)totalRes / N;

    // ==================== SIMPLEX BASELINE (same simulation, sequential) ====================
    long simplexTotalLat = 0, simplexTotalRes = 0;

    for (int pkt = 0; pkt < N; pkt++) {
        set<int> dests = {4, 5, 6};
        long lat = 0, res = 0;

        // Hop 1: same as FD
        set<int> activeFwds;
        bool covered = false;
        int maxH1 = 100;
        while (!covered && maxH1-- > 0) {
            lat++; res++;
            float randVal = randomgeneration(global_seed);
            double cum = 0.0;
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
            for (int f : got) activeFwds.insert(f);
            bool canCover4 = activeFwds.count(1);
            bool canCover6 = activeFwds.count(2) || activeFwds.count(3);
            covered = canCover4 && canCover6;
        }

        // Hop 2: SIMPLEX — optimal relay selection (single best relay per slot)
        vector<int> fwdList(activeFwds.begin(), activeFwds.end());
        int maxH2 = 1000;
        while (!dests.empty() && maxH2-- > 0) {
            int cov = 0;
            if (dests.count(4)) cov |= 1;
            if (dests.count(5)) cov |= 2;
            if (dests.count(6)) cov |= 4;

            int bestFwd = fwdList[0];
            double bestCost = 1e18;
            for (int fwd : fwdList) {
                double cost = relayCostAtState(fwd, cov,
                    p14, p15, p145, p25, p26, p256, p35, p36, p356);
                if (cost < bestCost) { bestCost = cost; bestFwd = fwd; }
            }

            lat++; res++;

            set<int> received;
            if (bestFwd == 1) received = simHop2(global_seed, p14, p15, p145, 4, 5);
            else if (bestFwd == 2) received = simHop2(global_seed, p25, p26, p256, 5, 6);
            else received = simHop2(global_seed, p35, p36, p356, 5, 6);

            for (int d : received) dests.erase(d);
        }

        simplexTotalLat += lat;
        simplexTotalRes += res;
    }

    double sAvgLat = (double)simplexTotalLat / N;
    double sAvgRes = (double)simplexTotalRes / N;

    // ==================== OUTPUT ====================
    cout << "\n============================================================\n";
    cout << "  SIMULATION RESULTS (" << N << " packets)\n";
    cout << "============================================================\n";

    cout << "\nSimplex (one forwarder per slot):\n";
    cout << "  Avg Latency  (slots) = " << sAvgLat << "\n";
    cout << "  Avg Resource (tx)    = " << sAvgRes << "\n";

    cout << "\nFull Duplex (all forwarders per slot):\n";
    cout << "  Avg Latency  (slots) = " << avgLat << "\n";
    cout << "  Avg Resource (tx)    = " << avgRes << "\n";

    if (sAvgLat > EPS && avgLat > EPS) {
        double latRed = (sAvgLat - avgLat) / sAvgLat * 100.0;
        double resInc = (avgRes - sAvgRes) / sAvgRes * 100.0;
        cout << "\n--- Comparison ---\n";
        cout << "                    Simplex    Full Duplex  Change\n";
        cout << "Latency  (slots)  " << setw(10) << sAvgLat
             << "  " << setw(10) << avgLat
             << "  " << latRed << "% less\n";
        cout << "Resource (tx)     " << setw(10) << sAvgRes
             << "  " << setw(10) << avgRes
             << "  " << resInc << "% more\n";
    }

    cout << "\n============================================================\n";
    return 0;
}
