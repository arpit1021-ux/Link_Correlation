/*
 * Analytical (exact, no RNG) validation for Phase 4's topology:
 *   0 -> {1,2,3} -> {4,5,6}, multicast (all three destinations required)
 *   node1 -> {4,5}, node2 -> {5,6}, node3 -> {5,6}
 * Uses the SAME bitmask coverage-state engine validated in Phase 3.5,
 * with the corrected R1/R2/R3 building block (no +1 bugs, no wrong-branch
 * substitutions), and given joint probabilities (inclusion-exclusion,
 * not copula - exact input data, same method as paper_validation.cpp).
 */
#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(5);

    cout << "============================================================\n";
    cout << "  Phase 4 Analytical: Multicast Cost (Bitmask Engine)\n";
    cout << "  Topology: 0 -> {1,2,3} -> {4,5,6}\n";
    cout << "============================================================\n\n";

    // ---- Hop-1: node0 -> {1,2,3} ----
    double p01, p02, p03;
    double p012, p013, p023, p0123;

    cout << "Hop-1 probabilities:\n";
    cout << "Enter p(0->1) p(0->2) p(0->3): ";
    cin >> p01 >> p02 >> p03;
    cout << "Enter p(0->1,2) p(0->1,3) p(0->2,3) p(0->1,2,3): ";
    cin >> p012 >> p013 >> p023 >> p0123;

    // ---- Hop-2: each relay's own two-destination joint distribution ----
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

    // beta_fa: exact probability of each of the 8 reception subsets over {1,2,3}
    map<int,double> beta;
    beta[0b000] = 1 - p01 - p02 - p03 + p012 + p013 + p023 - p0123;
    beta[0b001] = p01 - p012 - p013 + p0123; // only 1
    beta[0b010] = p02 - p012 - p023 + p0123; // only 2
    beta[0b100] = p03 - p013 - p023 + p0123; // only 3
    beta[0b011] = p012 - p0123; // 1,2 only
    beta[0b101] = p013 - p0123; // 1,3 only
    beta[0b110] = p023 - p0123; // 2,3 only
    beta[0b111] = p0123;        // all three

    cout << "\n=== Phase A region probabilities ===\n";
    double sum=0;
    for (auto &kv : beta) { cout << "  " << bitset<3>(kv.first) << " : " << kv.second << "\n"; sum+=kv.second; if(kv.second<-1e-9) cout<<"  *** NEGATIVE - inputs inconsistent ***\n"; }
    cout << "  sum = " << sum << " (should be 1.0)\n\n";

    // absorbing condition: need node1 (bit0) AND (node2(bit1) OR node3(bit2))
    auto absorbedA = [](int s){ return (s&1) && (s&6); };
    vector<int> transient = {0b000,0b001,0b010,0b100,0b110};
    vector<int> absorbH   = {0b011,0b101,0b111};

    // Phase A: per-transient-state expected transmissions (cost) - fixed point
    map<int,double> costA; for (int s: transient) costA[s]=5.0;
    for (int iter=0; iter<3000; iter++) {
        map<int,double> nc;
        for (int s : transient) {
            double acc = 1.0;
            for (auto &kv : beta) {
                int ns = s | kv.first;
                if (absorbedA(ns)) continue;
                acc += kv.second * costA[ns];
            }
            nc[s]=acc;
        }
        double diff=0; for(auto&kv:nc) diff=max(diff,fabs(kv.second-costA[kv.first]));
        costA=nc;
        if (diff<1e-10) break;
    }
    // Phase A: per-transient-state distribution over absorbing H
    map<int,map<int,double>> distA; for (int s: transient) for (int H: absorbH) distA[s][H]=1.0/3;
    for (int iter=0;iter<3000;iter++){
        map<int,map<int,double>> nd;
        for (int s: transient){
            map<int,double> acc; for (int H:absorbH) acc[H]=0;
            for (auto &kv: beta){
                int ns = s|kv.first;
                if (absorbedA(ns)) acc[ns]+=kv.second;
                else for (int H:absorbH) acc[H]+= kv.second*distA[ns][H];
            }
            nd[s]=acc;
        }
        distA=nd;
    }

    cout << "=== Phase A: per-transient-state cost and absorption distribution ===\n";
    for (int s: transient) {
        cout << "  state " << bitset<3>(s) << ": cost=" << costA[s] << "  dist: ";
        for (int H: absorbH) cout << bitset<3>(H) << "=" << distA[s][H] << " ";
        cout << "\n";
    }
    cout << "\n";

    // ---- Phase B: downstream cost per relay set H, bitmask coverage engine ----
    // destination bits: 4->bit0, 5->bit1, 6->bit2
    auto relayReach = [](int relay)->int{ return (relay==1)?0b011:0b110; };
    // local outcome dist for a relay: returns vector<(localmask,prob)> over its own 2 links
    auto localDist = [&](int relay)->vector<pair<int,double>>{
        double pA,pB,pAB;
        if (relay==1){pA=p14;pB=p15;pAB=p145;}
        else if (relay==2){pA=p25;pB=p26;pAB=p256;}
        else {pA=p35;pB=p36;pAB=p356;}
        double onlyA=pA-pAB, onlyB=pB-pAB, both=pAB, none=1-pA-pB+pAB;
        return {{0b00,none},{0b01,onlyA},{0b10,onlyB},{0b11,both}};
    };
    auto toGlobal = [&](int relay,int local)->int{
        int g=0;
        if (relay==1){ if(local&1) g|=1; if(local&2) g|=2; } // ->4,5
        else { if(local&1) g|=2; if(local&2) g|=4; }          // ->5,6
        return g;
    };

    auto downstreamCost = [&](int H, bool countPerNode)->double{
        vector<int> relays; for (int r:{1,2,3}) if (H&(1<<(r-1))) relays.push_back(r);
        map<int,double> c; for (int cov=0; cov<7; cov++) c[cov]=5.0;
        for (int iter=0; iter<3000; iter++){
            map<int,double> nc;
            for (int cov=0; cov<7; cov++){
                vector<int> active;
                for (int r: relays) if ((relayReach(r) & ~cov)!=0) active.push_back(r);
                double stepCost = countPerNode ? (double)active.size() : 1.0;
                double acc=stepCost;
                vector<pair<int,double>> dist = {{0,1.0}};
                for (int r: active) {
                    auto ld = localDist(r);
                    vector<pair<int,double>> nd;
                    for (auto &d: dist) for (auto &l: ld) {
                        int g = d.first | toGlobal(r,l.first);
                        nd.push_back({g, d.second*l.second});
                    }
                    dist = nd;
                }
                map<int,double> merged; for (auto&d:dist) merged[d.first]+=d.second;
                for (auto &kv: merged){
                    int newCov = cov | kv.first;
                    if (newCov==0b111) continue;
                    acc += kv.second * c[newCov];
                }
                nc[cov]=acc;
            }
            double diff=0; for(auto&kv:nc) diff=max(diff,fabs(kv.second-c[kv.first]));
            c=nc;
            if (diff<1e-10) break;
        }
        return c[0];
    };

    cout << "=== Phase B: downstream cost per absorbing relay set H (SLOTS metric) ===\n";
    map<int,double> PB, PBtx;
    for (int H: absorbH) { PB[H]=downstreamCost(H,false); cout << "  H=" << bitset<3>(H) << " : " << PB[H] << "\n"; }
    cout << "\n=== Phase B: downstream cost per absorbing relay set H (TX/resource metric) ===\n";
    for (int H: absorbH) { PBtx[H]=downstreamCost(H,true); cout << "  H=" << bitset<3>(H) << " : " << PBtx[H] << "\n"; }
    cout << "\n";

    // ---- Combine per-transient-state, properly (no lumping) ----
    map<int,double> E;
    // iterative solve since states reference each other (000 -> 001,010,100,110 etc.)
    for (int s: transient) E[s]=5.0;
    for (int iter=0;iter<3000;iter++){
        map<int,double> nE;
        for (int s: transient){
            double acc=1.0;
            for (auto &kv: beta){
                int ns = s|kv.first;
                if (absorbedA(ns)) acc += kv.second * PB[ns];
                else acc += kv.second * E[ns];
            }
            nE[s]=acc;
        }
        double diff=0; for(auto&kv:nE) diff=max(diff,fabs(kv.second-E[kv.first]));
        E=nE;
        if (diff<1e-10) break;
    }

    cout << "=== Final per-state E (Phase A + downstream, properly combined) ===\n";
    for (int s: transient) cout << "  E[" << bitset<3>(s) << "] = " << E[s] << "\n";
    cout << "\nTOTAL (from state 000) SLOTS metric = " << E[0b000] << "\n";

    // total-transmissions version: step cost = number of ACTIVE relays at each state
    // Phase A: step cost = 1 (source only). Phase B: step cost = N_active relays.
    map<int,double> Etx;
    for (int s: transient) Etx[s]=5.0;
    for (int iter=0;iter<3000;iter++){
        map<int,double> nE;
        for (int s: transient){
            // Phase A: source broadcasts alone → 1 tx per broadcast
            double acc = 1.0;
            for (auto &kv: beta){
                int ns = s|kv.first;
                if (absorbedA(ns)) {
                    // Phase B: relay set = ns, count active relays at coverage state 0
                    int activeCount = 0;
                    for (int r:{1,2,3}) {
                        if (ns & (1<<(r-1))) {
                            if ((relayReach(r) & ~0) != 0) activeCount++;  // cov=0, all destinations uncovered
                        }
                    }
                    // PBtx[ns] already accounts for per-state active relay counts downstream
                    // But the FIRST relay transmission from ns costs activeCount, not 1
                    // Since PBtx starts from cov=0 and includes stepCost internally,
                    // we just add PBtx[ns] directly (its first step already uses correct stepCost)
                    acc += kv.second * PBtx[ns];
                }
                else acc += kv.second * Etx[ns];
            }
            nE[s]=acc;
        }
        double diff=0; for(auto&kv:nE) diff=max(diff,fabs(kv.second-Etx[kv.first]));
        Etx=nE;
        if (diff<1e-10) break;
    }
    cout << "\nTOTAL (from state 000) TX/resource metric = " << Etx[0b000] << "\n";

    return 0;
}
