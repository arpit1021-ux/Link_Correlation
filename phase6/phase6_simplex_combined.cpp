/*
 * Phase 6 — SIMPLEX Combined Engine (generic topology, from user input)
 * ------------------------------------------------------------------------
 * Takes ANY acyclic multi-hop topology as INPUT (no hardcoded network
 * anywhere) - node count, source, destination set, edges (as ETX costs),
 * and correlation data are all read at runtime. Computes:
 *   1. The full analytical routing table: cost(node, destination-subset)
 *      for every node and every non-empty subset of destinations, via the
 *      validated Bellman-Ford relaxation + accumulation-state engine.
 *   2. A discrete-event SIMPLEX simulation of the same topology.
 *   3. A side-by-side comparison (analytical vs simulated, % error) for
 *      the actual question that matters: source -> full destination set.
 *   4. A readable routing-table dump and a "what does the source's first
 *      broadcast attempt actually decide" breakdown, for transparency
 *      about the (adaptive, not single-upfront-choice) forwarding policy.
 *
 * INPUT FORMAT (whitespace/newline separated, no comments in the data
 * itself - see accompanying README for a commented walkthrough):
 *
 *   N S                     <- number of nodes, source node id
 *   D                       <- number of destinations (supports up to 4)
 *   d1 d2 ... dD            <- destination node ids
 *   For each node i = 0..N-1, in order:
 *     k                     <- out-degree of node i (0 for a destination/sink)
 *     n1 c1 n2 c2 ... nk ck <- k pairs: (neighbor id, ETX cost of that link)
 *   For each node i = 0..N-1 with out-degree >= 2, in order:
 *     C                     <- number of correlation entries given (0 = treat
 *                              all pairs among this node's links as independent)
 *     C lines, each:
 *       m id_1 ... id_m v   <- P(all of these m neighbor-ids succeed together) = v
 *   lambda targetPackets    <- Poisson arrival rate, packets to simulate
 */
#include <bits/stdc++.h>
using namespace std;

struct EdgeIn { int to; double cost; double p; };

int main(){
    int N, S;
    cout << "Enter number of nodes N, source node S: ";
    cin >> N >> S;

    int D;
    cout << "Enter number of destinations D: ";
    cin >> D;
    if (D > 4) { cerr << "ERROR: this engine supports at most 4 destinations (bitmask size).\n"; return 1; }
    vector<int> destList(D);
    cout << "Enter " << D << " destination node ids: ";
    for (auto &d : destList) cin >> d;
    map<int,int> destBit;
    for (int i=0;i<D;i++) destBit[destList[i]] = 1<<i;
    int FULL = (1<<D)-1;
    auto reach = [&](int v)->int{ return destBit.count(v)? destBit[v] : 0; };

    map<int, vector<EdgeIn>> adj;
    cout << "Now enter each node's out-edges (node 0.." << N-1 << " in order).\n";
    for (int i=0;i<N;i++) {
        int k;
        cout << "Node " << i << ": out-degree k, then k pairs (neighbor cost): ";
        cin >> k;
        vector<EdgeIn> edges(k);
        for (auto &e : edges) {
            cin >> e.to >> e.cost;
            if (e.cost <= 0) { cerr << "ERROR: node " << i << " has non-positive cost - invalid.\n"; return 1; }
            e.p = 1.0/e.cost;
            if (e.p > 1.0) { cerr << "ERROR: node " << i << "->" << e.to << " implies probability > 1 (cost < 1).\n"; return 1; }
        }
        adj[i] = edges;
    }

    map<int, map<vector<int>,double>> corr;
    cout << "Now enter correlation data for each node with out-degree >= 2.\n";
    for (int i=0;i<N;i++) {
        if (adj[i].size() < 2) continue;
        int C;
        cout << "Node " << i << " (out-degree " << adj[i].size() << "): number of correlation entries C (0 = independent): ";
        cin >> C;
        for (int c=0;c<C;c++) {
            int m; cin >> m;
            vector<int> ids(m);
            for (auto &id : ids) cin >> id;
            double v; cin >> v;
            if (v < 0 || v > 1) { cerr << "ERROR: correlation value out of [0,1] for node " << i << ".\n"; return 1; }
            sort(ids.begin(), ids.end());
            corr[i][ids] = v;
        }
    }

    long long TARGET;
    cout << "Enter target number of packets to simulate: ";
    cin >> TARGET;

    // ---- build full joint outcome distribution per node (Mobius inversion) ----
    map<int, map<int,double>> pExactly;
    map<int, vector<pair<int,double>>> outcomeDistFlat; // for simulation sampling
    for (auto &kv : adj) {
        int node = kv.first;
        auto &edges = kv.second;
        int k = edges.size();
        if (k==0) continue;
        vector<int> parent(k); iota(parent.begin(),parent.end(),0);
        function<int(int)> find = [&](int x){ return parent[x]==x?x:parent[x]=find(parent[x]); };
        auto uni = [&](int a,int b){ a=find(a);b=find(b); if(a!=b) parent[a]=b; };
        auto idxOf = [&](int nb)->int{ for(int i=0;i<k;i++) if(edges[i].to==nb) return i; return -1; };
        if (corr.count(node)) for (auto &c: corr[node]) {
            if (c.first.size()<2) continue;
            vector<int> idxs; for (int nb: c.first) idxs.push_back(idxOf(nb));
            for (size_t i=1;i<idxs.size();i++) uni(idxs[0], idxs[i]);
        }
        auto pAtLeast = [&](int mask)->double{
            if (mask==0) return 1.0;
            map<int,vector<int>> byComp;
            for (int i=0;i<k;i++) if (mask&(1<<i)) byComp[find(i)].push_back(i);
            double result=1.0;
            for (auto &comp: byComp) {
                auto &li = comp.second;
                if (li.size()==1) { result *= edges[li[0]].p; continue; }
                vector<int> ids; for (int l: li) ids.push_back(edges[l].to);
                sort(ids.begin(),ids.end());
                if (corr[node].count(ids)) { result *= corr[node][ids]; continue; }
                double prod=1.0; for (int l: li) prod *= edges[l].p;
                result *= prod;
            }
            return result;
        };
        for (int T=0; T<(1<<k); T++) {
            double acc=0; int rest=((1<<k)-1)&~T;
            for (int R=rest; ; R=(R-1)&rest) {
                int U=T|R; int bits=__builtin_popcount(R);
                acc += ((bits%2==0)?1:-1)*pAtLeast(U);
                if (R==0) break;
            }
            pExactly[node][T]=acc;
        }
        double sum=0; for (auto&kv2:pExactly[node]) sum+=kv2.second;
        if (fabs(sum-1.0)>1e-6) cerr << "WARNING: node "<<node<<" outcome probs sum to "<<sum<<" (should be 1.0) - check correlation inputs.\n";
        for (auto&kv2:pExactly[node]) {
            if (kv2.second < -1e-9) cerr << "WARNING: node "<<node<<" has NEGATIVE probability "<<kv2.second<<" - correlation inputs violate Frechet bounds.\n";
            if (kv2.second > 1e-12) outcomeDistFlat[node].push_back({kv2.first, kv2.second});
        }
    }

    // ============================================================
    // PART 1: ANALYTICAL - Bellman-Ford relaxation + accumulation engine
    // ============================================================
    vector<int> allNodes; for (auto&kv:adj) allNodes.push_back(kv.first);
    map<int, vector<double>> cost; // cost[v][need], need in 0..FULL
    for (int v: allNodes) { cost[v].assign(FULL+1, 1e18); cost[v][0]=0.0; }
    const double INF=1e18;

    for (int iter=0; iter<1000; iter++) {
        double maxDelta=0;
        for (int v: allNodes) {
            for (int need=1; need<=FULL; need++) {
                int effNeed = need & ~reach(v);
                if (effNeed==0) {
                    if (cost[v][need] > 1e-9) maxDelta = max(maxDelta, cost[v][need]);
                    cost[v][need]=0;
                    continue;
                }
                auto &edges = adj[v];
                int k = edges.size();
                if (k==0) continue;

                int nStates = 1<<k;
                vector<double> V(nStates, -1.0);
                function<double(int)> solve = [&](int acc)->double{
                    if (V[acc] >= 0) return V[acc];
                    V[acc] = 1e17;
                    int coveredBits=0;
                    for (int i=0;i<k;i++) if (acc&(1<<i)) coveredBits |= reach(edges[i].to);
                    int remaining = effNeed & ~coveredBits;
                    double stopVal = INF;
                    if (remaining==0) stopVal = 0.0;
                    else {
                        // ---- Method 1: dynamic coverage-race - for
                        // accumulated relays that connect DIRECTLY to final
                        // destinations, keep re-evaluating which relay is
                        // best as coverage evolves, rather than locking in
                        // an upfront assignment. ----
                        vector<int> terminalRelays;
                        for (int i=0;i<k;i++) if (acc&(1<<i)) {
                            int r = edges[i].to;
                            bool allDestNeighbors = adj.count(r) && !adj[r].empty();
                            if (allDestNeighbors) for (auto &e2 : adj[r]) if (reach(e2.to)==0) { allDestNeighbors=false; break; }
                            if (allDestNeighbors) terminalRelays.push_back(r);
                        }
                        double stopValDynamic = INF;
                        if (!terminalRelays.empty()) {
                            int nCov = 1<<D;
                            vector<double> cc(nCov, 5.0);
                            for (int it2=0; it2<3000; it2++) {
                                vector<double> nc(nCov);
                                for (int cov=0; cov<nCov; cov++) {
                                    int stillNeeded = effNeed & ~cov;
                                    if (stillNeeded==0) { nc[cov]=0; continue; }
                                    vector<int> active;
                                    for (int r : terminalRelays) {
                                        int rReach=0; for (auto &e2: adj[r]) rReach |= reach(e2.to);
                                        if (rReach & stillNeeded) active.push_back(r);
                                    }
                                    if (active.empty()) { nc[cov]=INF; continue; }
                                    double best=INF;
                                    for (int r : active) {
                                        double a=1.0;
                                        map<int,double> merged;
                                        for (auto &kv2 : pExactly[r]) {
                                            int g=0; auto &redges=adj[r];
                                            for (int i2=0;i2<(int)redges.size();i2++) if (kv2.first&(1<<i2)) g|=reach(redges[i2].to);
                                            merged[cov|g] += kv2.second;
                                        }
                                        for (auto &kv2 : merged) {
                                            a += kv2.second * cc[kv2.first];
                                        }
                                        best = min(best, a);
                                    }
                                    nc[cov]=best;
                                }
                                double diff=0; for (int cov=0;cov<nCov;cov++) {
                                    double a=nc[cov], b=cc[cov];
                                    if (a>=INF && b>=INF) continue;
                                    diff=max(diff, fabs(a-b));
                                }
                                cc=nc;
                                if (diff<1e-9) break;
                            }
                            stopValDynamic = cc[coveredBits];
                        }

                        // ---- Method 2: one-shot partition search (safety
                        // net fallback for deeper multi-hop relays) ----
                        double stopValPartition = INF;
                        vector<int> bits; for (int b=0;b<D;b++) if (remaining & (1<<b)) bits.push_back(1<<b);
                        int nb = bits.size();
                        vector<int> assign(nb, 0);
                        function<void(int,int)> gen = [&](int idx, int maxLabel){
                            if (idx==nb) {
                                map<int,int> parts;
                                for (int i=0;i<nb;i++) parts[assign[i]] |= bits[i];
                                double sum=0; bool bad=false;
                                for (auto &p : parts) {
                                    double best=INF;
                                    for (int i=0;i<k;i++) if (acc&(1<<i)) {
                                        int r = edges[i].to;
                                        double c = (reach(r)&p.second)==p.second ? 0.0 : cost[r][p.second];
                                        best = min(best, c);
                                    }
                                    if (best>=INF) { bad=true; break; }
                                    sum += best;
                                }
                                if (!bad) stopValPartition = min(stopValPartition, sum);
                                return;
                            }
                            for (int lbl=0; lbl<=maxLabel; lbl++) { assign[idx]=lbl; gen(idx+1, max(maxLabel, lbl+1)); }
                        };
                        gen(0,0);

                        stopVal = min(stopValDynamic, stopValPartition);
                    }
                    double contVal = INF;
                    {
                        double pStay=0.0;
                        for (auto &kv2: pExactly[v]) if ((acc|kv2.first)==acc) pStay += kv2.second;
                        if (pStay < 1.0-1e-12) {
                            double numer=1.0; bool bad=false;
                            for (auto &kv2: pExactly[v]) {
                                int nextAcc = acc | kv2.first;
                                if (nextAcc==acc) continue;
                                double childVal = solve(nextAcc);
                                if (childVal>=INF) { bad=true; break; }
                                numer += kv2.second * childVal;
                            }
                            if (!bad) contVal = numer/(1.0-pStay);
                        }
                    }
                    double result = min(stopVal, contVal);
                    V[acc]=result;
                    return result;
                };
                double best = solve(0);
                if (best < cost[v][need]) {
                    maxDelta = max(maxDelta, cost[v][need]-best);
                    cost[v][need] = best;
                }
            }
        }
        if (maxDelta < 1e-9) { cerr << "[Analytical] converged at iteration " << iter << "\n"; break; }
    }

    cout << fixed << setprecision(4);
    cout << "\n=== ANALYTICAL ROUTING TABLE (SIMPLEX) ===\n";
    cout << "Destination bit map: ";
    for (int i=0;i<D;i++) cout << destList[i] << "=bit" << i << " ";
    cout << "\n\n";
    for (int v : allNodes) {
        cout << "Node " << v << ": ";
        for (int need=1; need<=FULL; need++) {
            double c = cost[v][need];
            cout << "need=" << bitset<4>(need).to_string().substr(4-D) << ":" << (c>=INF?string("INF"):[&]{std::ostringstream o;o<<c;return o.str();}()) << "  ";
        }
        cout << "\n";
    }

    double analyticalAnswer = cost[S][FULL];
    cout << "\n>>> HEADLINE RESULT: cost(source=" << S << ", ALL destinations) = "
         << (analyticalAnswer>=INF? string("INF") : [&]{std::ostringstream o;o<<analyticalAnswer;return o.str();}()) << " <<<\n";

    // First-broadcast breakdown, for transparency about the adaptive policy
    cout << "\n--- Source's first-broadcast outcome breakdown (illustrative, not a fixed upfront choice) ---\n";
    if (adj.count(S) && !adj[S].empty()) {
        for (auto &kv2 : pExactly[S]) {
            if (kv2.first==0) continue;
            string s="{";
            for (int i=0;i<(int)adj[S].size();i++) if (kv2.first&(1<<i)) s+=to_string(adj[S][i].to)+",";
            if (s.back()==',') s.pop_back(); s+="}";
            cout << "  P(receive " << s << ") = " << kv2.second << "\n";
        }
    }

    // ============================================================
    // PART 2: SIMULATION - one packet at a time, no queueing.
    // Reuses the SAME accumulate-until-structurally-sufficient logic as
    // the analytical engine (a single "holder" is NOT enough in general -
    // topologies where two genuinely distinct relay branches are each
    // mandatory for different destinations need the packet to wait for
    // BOTH to be acquired before proceeding, exactly like the analytical
    // model's accumulation state machine).
    // ============================================================
    static mt19937_64 rng(20260729);
    static uniform_real_distribution<double> U(0.0,1.0);
    auto sampleFrom = [&](vector<pair<int,double>>& dist)->int{
        double r=U(rng), cum=0;
        for (auto&kv2:dist) { cum+=kv2.second; if (r<=cum) return kv2.first; }
        return dist.back().first;
    };

    // simulate node v delivering `need` (recursive - v accumulates until
    // sufficient, then hands off each part sequentially to its best relay)
    function<int(int,int)> simulateNode = [&](int v, int need)->int{
        int effNeed = need & ~reach(v);
        if (effNeed==0) return 0;
        if (!adj.count(v) || adj[v].empty()) return 1000000; // dead end, shouldn't be reached if feasible

        int k = adj[v].size();
        int acc = 0;
        int tx = 0;
        while (true) {
            tx++;
            int mask = outcomeDistFlat.count(v) ? sampleFrom(outcomeDistFlat[v]) : 0;
            acc |= mask;
            int coveredBits=0;
            for (int i=0;i<k;i++) if (acc&(1<<i)) coveredBits |= reach(adj[v][i].to);
            int remaining = effNeed & ~coveredBits;
            if (remaining==0) return tx;

            // is the accumulated set structurally sufficient? (every
            // remaining bit has some accumulated relay with finite cost)
            vector<int> bits; for (int b=0;b<D;b++) if (remaining&(1<<b)) bits.push_back(1<<b);
            int nb = bits.size();
            double bestSum = 1e18;
            vector<pair<int,int>> bestAssignment; // (partMask, relayId)
            vector<int> assign(nb,0);
            function<void(int,int)> gen = [&](int idx,int maxLabel){
                if (idx==nb) {
                    map<int,int> parts;
                    for (int i=0;i<nb;i++) parts[assign[i]] |= bits[i];
                    double sum=0; bool bad=false;
                    vector<pair<int,int>> thisAssignment;
                    for (auto &p : parts) {
                        double best=1e18; int bestRelay=-1;
                        for (int i=0;i<k;i++) if (acc&(1<<i)) {
                            int r = adj[v][i].to;
                            double c = (reach(r)&p.second)==p.second ? 0.0 : cost[r][p.second];
                            if (c < best) { best=c; bestRelay=r; }
                        }
                        if (bestRelay==-1) { bad=true; break; }
                        sum += best;
                        thisAssignment.push_back({p.second, bestRelay});
                    }
                    if (!bad && sum < bestSum) { bestSum = sum; bestAssignment = thisAssignment; }
                    return;
                }
                for (int lbl=0; lbl<=maxLabel; lbl++) { assign[idx]=lbl; gen(idx+1, max(maxLabel,lbl+1)); }
            };
            gen(0,0);

            if (bestSum < 1e17) {
                // Prefer the dynamic coverage-race (matches validated
                // methodology) whenever the accumulated relays needed are
                // "terminal" (connect directly to final destinations) -
                // re-evaluate which relay is best as coverage evolves,
                // rather than committing to a fixed upfront assignment.
                vector<int> terminalRelays;
                for (int i=0;i<k;i++) if (acc&(1<<i)) {
                    int r = adj[v][i].to;
                    bool allDest = adj.count(r) && !adj[r].empty();
                    if (allDest) for (auto &e2: adj[r]) if (reach(e2.to)==0) { allDest=false; break; }
                    if (allDest) terminalRelays.push_back(r);
                }
                // can terminal relays alone structurally cover `remaining`?
                bool terminalSufficient = true;
                {
                    int coverableByTerminal=0;
                    for (int r: terminalRelays) for (auto &e2: adj[r]) coverableByTerminal |= reach(e2.to);
                    if ((remaining & ~coverableByTerminal) != 0) terminalSufficient = false;
                }
                if (!terminalRelays.empty() && terminalSufficient) {
                    // recompute the same round-by-round value function the
                    // analytical dynamic race uses, so relay selection here
                    // matches exactly (no relay needs to finish everything
                    // alone - just contribute progress each round)
                    int nCov = 1<<D;
                    vector<double> cc(nCov, 5.0);
                    for (int it2=0; it2<3000; it2++) {
                        vector<double> nc(nCov);
                        for (int cov=0; cov<nCov; cov++) {
                            int stillNeeded2 = remaining & ~cov;
                            if (stillNeeded2==0) { nc[cov]=0; continue; }
                            vector<int> active;
                            for (int r: terminalRelays) {
                                int rReach=0; for (auto &e2: adj[r]) rReach |= reach(e2.to);
                                if (rReach & stillNeeded2) active.push_back(r);
                            }
                            if (active.empty()) { nc[cov]=INF; continue; }
                            double best=INF;
                            for (int r: active) {
                                double a=1.0;
                                map<int,double> merged;
                                for (auto &kv2: pExactly[r]) {
                                    int g=0; auto &redges=adj[r];
                                    for (int i2=0;i2<(int)redges.size();i2++) if (kv2.first&(1<<i2)) g|=reach(redges[i2].to);
                                    merged[cov|g] += kv2.second;
                                }
                                for (auto &kv2: merged) a += kv2.second * cc[kv2.first];
                                best = min(best, a);
                            }
                            nc[cov]=best;
                        }
                        double diff=0; for (int c2=0;c2<nCov;c2++) { double a=nc[c2],b=cc[c2]; if(a>=INF&&b>=INF) continue; diff=max(diff,fabs(a-b)); }
                        cc=nc;
                        if (diff<1e-9) break;
                    }

                    int covered2 = 0;
                    int extra = 0;
                    while ((remaining & ~covered2) != 0) {
                        int stillNeeded2 = remaining & ~covered2;
                        int bestRelay=-1; double bestVal=1e18;
                        for (int r: terminalRelays) {
                            int rReach=0; for (auto &e2: adj[r]) rReach |= reach(e2.to);
                            if (!(rReach & stillNeeded2)) continue;
                            double a=1.0;
                            map<int,double> merged;
                            for (auto &kv2: pExactly[r]) {
                                int g=0; auto &redges=adj[r];
                                for (int i2=0;i2<(int)redges.size();i2++) if (kv2.first&(1<<i2)) g|=reach(redges[i2].to);
                                merged[covered2|g] += kv2.second;
                            }
                            for (auto &kv2: merged) a += kv2.second * cc[kv2.first];
                            if (a < bestVal) { bestVal=a; bestRelay=r; }
                        }
                        if (bestRelay==-1) break; // shouldn't happen given terminalSufficient
                        extra++;
                        int mask2 = outcomeDistFlat.count(bestRelay) ? sampleFrom(outcomeDistFlat[bestRelay]) : 0;
                        int g=0; auto &redges = adj[bestRelay];
                        for (int i2=0;i2<(int)redges.size();i2++) if (mask2&(1<<i2)) g|=reach(redges[i2].to);
                        covered2 |= g;
                    }
                    return tx + extra;
                }
                // fallback: one-shot partition assignment (deeper/mixed relays)
                int extra = 0;
                for (auto &pr : bestAssignment) extra += simulateNode(pr.second, pr.first);
                return tx + extra;
            }
            // else: not yet sufficient, keep accumulating (loop continues)
        }
    };

    vector<int> txCounts;
    for (long long pkt=0; pkt<TARGET; pkt++) {
        txCounts.push_back(simulateNode(S, FULL));
    }

    auto avgi=[](vector<int>&v){return v.empty()?0.0:accumulate(v.begin(),v.end(),0.0)/v.size();};

    cout << "\n=== SIMULATION (SIMPLEX, " << TARGET << " independent packets, no queueing) ===\n";
    double simAnswer = avgi(txCounts);
    cout << "Avg ETX (transmissions): " << simAnswer << "\n";

    // ============================================================
    // PART 3: FINAL COMPARISON
    // ============================================================
    cout << "\n=== FINAL COMPARISON: source " << S << " -> ALL destinations ===\n";
    cout << "Analytical : " << analyticalAnswer << "\n";
    cout << "Simulated  : " << simAnswer << "\n";
    if (analyticalAnswer < INF && analyticalAnswer > 1e-9) {
        double err = 100.0*(simAnswer-analyticalAnswer)/analyticalAnswer;
        cout << "Error      : " << err << "%\n";
        if (fabs(err) > 5.0) cout << "*** WARNING: error exceeds 5% - investigate before trusting this result ***\n";
    }
    return 0;
}
