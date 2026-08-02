// Phase 6: Generalized Multi-Hop Network
// Reads arbitrary topology from input file
// Uses iterative ETX calculation (bottom-up from destinations)
// Supports multi-hop paths, both simplex and full duplex models
//
// Algorithm:
// 1. Initialize: dest nodes have ETX=0 to themselves
// 2. For each layer backwards from destinations:
//    - For each node, compute ETX to each destination subset
//    - Use paper's formula with all non-empty forwarder subsets
// 3. Source's ETX to full destination set = final answer

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

// ====================== Data Structures ======================

struct Edge { int from, to; double cost; };

// ProbStore: marginal probabilities for each directed link
struct ProbStore {
    map<int, map<int, double>> marg;  // marg[src][dst] = P(src->dst)
};

// ====================== ETX Calculator ======================
// Uses iterative bottom-up approach (like reference project's Phase 7a)
// routingTable[node][destSubset] = ETX from node to cover destSubset
// bestFS[node][destSubset] = best forwarding set

class ETXCalculator {
    int numNodes;
    map<int, set<int>> adj;  // adjacency list (outgoing)
    map<int, set<int>> radj; // reverse adjacency (incoming)
    ProbStore prob;

    // routingTable[node][destSubset] = ETX cost
    map<int, map<set<int>, double>> routingTable;
    // bestFS[node][destSubset] = best forwarding set
    map<int, map<set<int>, set<int>>> bestFS;

    // Generate all non-empty subsets of a set
    vector<set<int>> getAllSubsets(const set<int>& s) {
        vector<int> elems(s.begin(), s.end());
        int n = elems.size();
        vector<set<int>> subsets;
        for (int mask = 1; mask < (1 << n); mask++) {
            set<int> subset;
            for (int i = 0; i < n; i++)
                if (mask & (1 << i)) subset.insert(elems[i]);
            subsets.push_back(subset);
        }
        return subsets;
    }

    // Compute ETX from node to destSet using forwarding set fs
    // Paper's formula: E = (1 + Σ P(exactly S) * X(S)) / (1 - P(none))
    double computeCost(int node, const set<int>& destSet, const set<int>& fs) {
        if (destSet.size() == 1 && destSet.count(node)) return 0.0;

        // Get neighbors in forwarding set that can REACH at least one destination
        vector<int> activeFwds;
        for (int f : fs) {
            if (!adj[node].count(f)) continue;
            // Check if this forwarder can reach any destination in destSet
            if (adj.count(f)) {
                for (int d : adj.at(f))
                    if (destSet.count(d)) { activeFwds.push_back(f); break; }
            }
        }

        if (activeFwds.empty()) return INF;

        int n = activeFwds.size();
        vector<double> p;
        for (int f : activeFwds)
            p.push_back(prob.marg[node][f]);

        // Enumerate all subsets of activeFwds
        double pnone = 1.0;
        for (int mask = 1; mask < (1 << n); mask++) {
            double pe = 1.0;
            for (int i = 0; i < n; i++) {
                if (mask & (1 << i)) pe *= p[i];
                else pe *= (1.0 - p[i]);
            }
            pnone -= pe;
        }

        if (pnone < -EPS) pnone = 0.0;
        double denom = 1.0 - pnone;
        if (denom < EPS) return INF;

        double num = 1.0;
        for (int mask = 1; mask < (1 << n); mask++) {
            double pe = 1.0;
            set<int> recv;
            for (int i = 0; i < n; i++) {
                if (mask & (1 << i)) { pe *= p[i]; recv.insert(activeFwds[i]); }
                else pe *= (1.0 - p[i]);
            }

            // Compute remaining cost for this receiver set
            // Simplex: pick best single forwarder from recv
            double bestCost = INF;
            for (int f : recv) {
                // Destinations this forwarder can reach directly
                set<int> covered;
                if (adj.count(f)) {
                    for (int d : adj.at(f))
                        if (destSet.count(d)) covered.insert(d);
                }
                // Remaining destinations
                set<int> remaining;
                for (int d : destSet)
                    if (!covered.count(d)) remaining.insert(d);

                double cost;
                if (remaining.empty()) {
                    cost = 0.0;
                } else if (routingTable.count(f) && routingTable[f].count(remaining)) {
                    cost = routingTable[f][remaining];
                } else {
                    // No route from this forwarder to remaining dests
                    cost = INF;
                }
                bestCost = min(bestCost, cost);
            }

            if (bestCost < INF)
                num += pe * bestCost;
        }

        return num / denom;
    }

public:
    ETXCalculator(int n) : numNodes(n) {}

    void addEdge(int from, int to, double cost) {
        adj[from].insert(to);
        radj[to].insert(from);
        prob.marg[from][to] = 1.0 / cost;
    }

    void computeRoutingTable(const set<int>& destinations) {
        // Initialize: dest nodes have ETX=0 to themselves
        for (int d : destinations) {
            set<int> single = {d};
            routingTable[d][single] = 0.0;
        }

        // Iterative: build routing tables bottom-up from destinations
        bool changed = true;
        int pass = 0;
        while (changed && pass < 50) {
            changed = false;
            pass++;

            // For each node that has edges to nodes with known routing tables
            for (int node = 0; node < numNodes; node++) {
                if (adj.empty() || !adj.count(node)) continue;

                // Get all destination subsets
                vector<set<int>> destSubsets = getAllSubsets(destinations);

                for (const auto& destSet : destSubsets) {
                    if (destSet.size() == 1 && destSet.count(node)) {
                        routingTable[node][destSet] = 0.0;
                        continue;
                    }

                    // Get all non-empty subsets of neighbors as candidate forwarding sets
                    set<int> neighbors;
                    for (int nb : adj[node]) neighbors.insert(nb);
                    vector<set<int>> allFS = getAllSubsets(neighbors);

                    double bestCost = INF;
                    set<int> bestForwardingSet;

                    for (const auto& fs : allFS) {
                        double cost = computeCost(node, destSet, fs);
                        if (cost < bestCost - EPS) {
                            bestCost = cost;
                            bestForwardingSet = fs;
                        }
                    }

                    if (bestCost < INF) {
                        double oldCost = routingTable[node].count(destSet) ?
                                         routingTable[node][destSet] : INF;
                        if (bestCost < oldCost - EPS) {
                            routingTable[node][destSet] = bestCost;
                            bestFS[node][destSet] = bestForwardingSet;
                            changed = true;
                        } else if (!routingTable[node].count(destSet)) {
                            routingTable[node][destSet] = bestCost;
                            bestFS[node][destSet] = bestForwardingSet;
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    double getETX(int source, const set<int>& destinations) {
        if (routingTable.count(source) && routingTable[source].count(destinations))
            return routingTable[source][destinations];
        return INF;
    }

    void printRoutingTable() {
        cout << "\n--- Routing Tables ---\n";
        for (auto& nodeEntry : routingTable) {
            int node = nodeEntry.first;
            for (auto& destEntry : nodeEntry.second) {
                const set<int>& destSet = destEntry.first;
                double cost = destEntry.second;
                cout << "  " << node << " -> {";
                for (auto it = destSet.begin(); it != destSet.end(); it++)
                    cout << (it != destSet.begin() ? "," : "") << *it;
                cout << "} = " << cost;
                if (bestFS[node].count(destSet)) {
                    cout << "  FS={";
                    for (auto it = bestFS[node][destSet].begin();
                         it != bestFS[node][destSet].end(); it++)
                        cout << (it != bestFS[node][destSet].begin() ? "," : "") << *it;
                    cout << "}";
                }
                cout << "\n";
            }
        }
    }
};

// ====================== Input Reader ======================
static bool getNextDataLine(ifstream &fin, string &line) {
    while (getline(fin, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        line = line.substr(start);
        if (line[0] == '#') continue;
        return true;
    }
    return false;
}

// ====================== Main ======================
int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    string inputFile = "phase6_input.txt";
    if (argc > 1) inputFile = argv[1];

    cout << "============================================================\n";
    cout << "  Phase 6: Generalized Multi-Hop Network\n";
    cout << "  Reading from: " << inputFile << "\n";
    cout << "============================================================\n\n";

    ifstream fin(inputFile);
    if (!fin.is_open()) {
        cerr << "ERROR: Could not open " << inputFile << "\n";
        return 1;
    }

    string line;
    int numNodes, numEdges, source, numDests;

    getNextDataLine(fin, line); numNodes = stoi(line);
    getNextDataLine(fin, line); source = stoi(line);
    getNextDataLine(fin, line); numDests = stoi(line);

    vector<int> destVec;
    getNextDataLine(fin, line);
    istringstream dss(line);
    for (int i = 0; i < numDests; i++) { int d; dss >> d; destVec.push_back(d); }
    set<int> destinations(destVec.begin(), destVec.end());

    cout << "Nodes: " << numNodes << "\n";
    cout << "Source: " << source << "\n";
    cout << "Destinations: {";
    for (auto it = destinations.begin(); it != destinations.end(); it++)
        cout << (it != destinations.begin() ? "," : "") << *it;
    cout << "}\n";

    getNextDataLine(fin, line); numEdges = stoi(line);
    cout << "Edges: " << numEdges << "\n";

    ETXCalculator calc(numNodes);

    for (int i = 0; i < numEdges; i++) {
        getNextDataLine(fin, line);
        istringstream ess(line);
        int from, to; double cost;
        ess >> from >> to >> cost;
        calc.addEdge(from, to, cost);
        cout << "  " << from << " -> " << to << " (cost=" << cost << ", p=" << 1.0/cost << ")\n";
    }
    fin.close();

    // Compute routing tables
    calc.computeRoutingTable(destinations);
    calc.printRoutingTable();

    double etx = calc.getETX(source, destinations);

    cout << "\n============================================================\n";
    cout << "  RESULT: ETX (source " << source << " -> {";
    for (auto it = destinations.begin(); it != destinations.end(); it++)
        cout << (it != destinations.begin() ? "," : "") << *it;
    cout << "})\n";
    cout << "  ETX = " << etx << "\n";
    cout << "============================================================\n";

    return 0;
}
