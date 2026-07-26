// Phase 3.5: Exact Paper Topology with Absorbing Condition
// Topology: S -> f1 -> D1, S -> f2 -> D2, S -> f3 -> D1, S -> f3 -> D2
// Multicast absorbing condition: (f1 OR f3 received) AND (f2 OR f3 received)
//
// Phase B uses bitmask coverage-state fixed-point engine:
//   covered = bitmask over destinations (bit0=D1, bit1=D2)
//   Each forwarder has a reach bitmask
//   A forwarder is active iff (reach & ~covered) != 0
//   At each state, build joint outcome distribution over active forwarders
//   Update covered, recurse until covered == full mask

#include <bits/stdc++.h>
using namespace std;

const double INF = 1e12;
const double EPS = 1e-12;

// ====================== STEP 1: Single-Forwarder Building Block ======================
struct FwdResult {
    double etx;
    double pNone, pOnlyA, pOnlyB, pBoth;
    double cA, cB;
};

FwdResult singleForwarderETX(double pA, double pB, double pAB) {
    FwdResult r;
    r.cA = (pA > EPS) ? 1.0 / pA : INF;
    r.cB = (pB > EPS) ? 1.0 / pB : INF;
    r.pOnlyA = max(0.0, pA - pAB);
    r.pOnlyB = max(0.0, pB - pAB);
    r.pBoth  = max(0.0, pAB);
    r.pNone  = max(0.0, 1.0 - pA - pB + pAB);
    double denom = 1.0 - r.pNone;
    if (denom < EPS) { r.etx = INF; return r; }
    double num = 1.0 + r.pOnlyA * r.cB + r.pOnlyB * r.cA;
    r.etx = num / denom;
    return r;
}

// ====================== CHECKPOINT A ======================
void checkpointA() {
    cout << "========================================\n";
    cout << "  CHECKPOINT A: f3 alone -> {D1, D2}\n";
    cout << "========================================\n";
    double p31 = 0.6, p32 = 0.6, p312 = 0.40;
    FwdResult r = singleForwarderETX(p31, p32, p312);
    cout << "  p(f3->D1) = " << p31 << "\n";
    cout << "  p(f3->D2) = " << p32 << "\n";
    cout << "  p(f3->D1,D2) = " << p312 << "\n\n";
    cout << "  Venn regions:\n";
    cout << "    P(none)  = " << r.pNone << "\n";
    cout << "    P(onlyD1)= " << r.pOnlyA << "\n";
    cout << "    P(onlyD2)= " << r.pOnlyB << "\n";
    cout << "    P(both)  = " << r.pBoth << "\n";
    cout << "    Sum      = " << (r.pNone + r.pOnlyA + r.pOnlyB + r.pBoth) << "\n\n";
    cout << "  Link costs: c(D1) = 1/" << p31 << " = " << r.cA << "\n";
    cout << "              c(D2) = 1/" << p32 << " = " << r.cB << "\n\n";
    cout << "  ETX(f3 -> {D1,D2}) = " << fixed << setprecision(4) << r.etx << "\n\n";
    double expected = 2.0834;
    if (fabs(r.etx - expected) < 0.001)
        cout << "  *** CHECKPOINT A PASSED *** (expected " << expected << ")\n";
    else
        cout << "  *** CHECKPOINT A FAILED *** (expected " << expected << ", got " << r.etx << ")\n";
    cout << "========================================\n\n";
}

// ====================== PHASE A: S -> {f1,f2,f3} ======================
struct PhaseAResult {
    double pNone, p1, p2, p3, p12, p13, p23, p123;
    double pAbsorb, pNonAbsorb;
    bool isAbsorb[8];
};

PhaseAResult computePhaseA(double p1, double p2, double p3,
                            double p12, double p13, double p23, double p123) {
    PhaseAResult r;
    r.p1   = max(0.0, p1 - p12 - p13 + p123);
    r.p2   = max(0.0, p2 - p12 - p23 + p123);
    r.p3   = max(0.0, p3 - p13 - p23 + p123);
    r.p12  = max(0.0, p12 - p123);
    r.p13  = max(0.0, p13 - p123);
    r.p23  = max(0.0, p23 - p123);
    r.p123 = max(0.0, p123);
    r.pNone = max(0.0, 1.0 - (p1+p2+p3) + (p12+p13+p23) - p123);

    // Absorbing: (f1 OR f3) AND (f2 OR f3) = f3 OR (f1 AND f2)
    r.isAbsorb[0] = false;  // none
    r.isAbsorb[1] = false;  // {f1}
    r.isAbsorb[2] = false;  // {f2}
    r.isAbsorb[3] = true;   // {f3}
    r.isAbsorb[4] = true;   // {f1,f2}
    r.isAbsorb[5] = true;   // {f1,f3}
    r.isAbsorb[6] = true;   // {f2,f3}
    r.isAbsorb[7] = true;   // {f1,f2,f3}

    double regions[8] = {r.pNone, r.p1, r.p2, r.p3, r.p12, r.p13, r.p23, r.p123};
    r.pAbsorb = 0.0; r.pNonAbsorb = 0.0;
    for (int i = 0; i < 8; i++) {
        if (r.isAbsorb[i]) r.pAbsorb += regions[i];
        else r.pNonAbsorb += regions[i];
    }
    return r;
}

void printPhaseA(const PhaseAResult& r) {
    cout << "  8-Region Probabilities:\n";
    cout << "    P(none)     = " << r.pNone << "\n";
    cout << "    P({f1})     = " << r.p1 << "\n";
    cout << "    P({f2})     = " << r.p2 << "\n";
    cout << "    P({f3})     = " << r.p3 << "\n";
    cout << "    P({f1,f2})  = " << r.p12 << "\n";
    cout << "    P({f1,f3})  = " << r.p13 << "\n";
    cout << "    P({f2,f3})  = " << r.p23 << "\n";
    cout << "    P({f1,f2,f3}) = " << r.p123 << "\n";
    cout << "    Sum = " << (r.pNone+r.p1+r.p2+r.p3+r.p12+r.p13+r.p23+r.p123) << "\n\n";
    cout << "  Absorbing condition: (f1 OR f3) AND (f2 OR f3)\n";
    cout << "    P(absorb)    = " << r.pAbsorb << "\n";
    cout << "    P(non-absorb)= " << r.pNonAbsorb << "\n";
}

// ====================== PHASE B: Bitmask Coverage-State Engine ======================

struct Forwarder {
    int id;
    int reachMask;           // bitmask of destinations this fwd can reach
    vector<pair<int, double>> outcomes;  // (coverage_mask, probability) for this fwd alone
};

// Build f3's outcome distribution from joint probabilities
// f3 reaches D1 (bit0) and D2 (bit1)
vector<pair<int, double>> buildF3Outcomes(double p31, double p32, double p312) {
    double pNone  = max(0.0, 1.0 - p31 - p32 + p312);  // 0b00
    double pOnly1 = max(0.0, p31 - p312);                // 0b01
    double pOnly2 = max(0.0, p32 - p312);                // 0b10
    double pBoth  = max(0.0, p312);                       // 0b11
    return {{0b00, pNone}, {0b01, pOnly1}, {0b10, pOnly2}, {0b11, pBoth}};
}

// Build single-link forwarder outcome distribution (Bernoulli)
// reachMask has exactly one bit set
vector<pair<int, double>> buildSingleOutcomes(double p) {
    return {{0b00, 1.0 - p}, {0b01, p}};  // only D1 reachable
}

// Build single-link forwarder for D2
vector<pair<int, double>> buildSingleOutcomesD2(double p) {
    return {{0b00, 1.0 - p}, {0b10, p}};  // only D2 reachable
}

// Bitmask coverage-state fixed-point engine
// covered: current bitmask of covered destinations
// fullMask: bitmask of all destinations (0b11 for {D1,D2})
// activeFwds: forwarders that are currently active (reach & ~covered) != 0
// Returns: expected transmissions from this state
double coverageStateETX(int covered, int fullMask,
                         const vector<Forwarder>& allFwds,
                         map<int, double>& memo) {
    if (covered == fullMask) return 0.0;  // all destinations covered
    if (memo.count(covered)) return memo[covered];

    // Determine which forwarders are active in this state
    vector<const Forwarder*> active;
    for (const auto& f : allFwds) {
        if ((f.reachMask & ~covered) != 0)  // has something to contribute
            active.push_back(&f);
    }

    if (active.empty()) return INF;  // no active forwarders — can't progress

    // Build joint outcome distribution over active forwarders
    // Each forwarder produces independent outcomes
    // Joint outcome = union of coverage from all active forwarders
    vector<pair<int, double>> joint = {{0, 1.0}};  // start with (mask=0, prob=1)

    for (const Forwarder* f : active) {
        vector<pair<int, double>> newJoint;
        for (auto& [jMask, jProb] : joint) {
            for (auto& [fMask, fProb] : f->outcomes) {
                newJoint.push_back({jMask | fMask, jProb * fProb});
            }
        }
        joint = newJoint;
    }

    // Aggregate joint outcomes by coverage mask
    map<int, double> outcomeProb;
    for (auto& [mask, prob] : joint) {
        // Only count coverage of still-uncovered destinations
        int effectiveMask = mask & ~covered;
        outcomeProb[effectiveMask] += prob;
    }

    // Solve: E = 1 + Σ P(outcome) * E(covered | outcome)
    // => E = (1 + Σ P(outcome for non-absorbing states) * E(next_state)) / (1 - P(stay))
    double pStay = 0.0;  // probability of staying in same state (no new coverage)
    double sumTransition = 0.0;

    for (auto& [mask, prob] : outcomeProb) {
        if (mask == 0) {
            // No new coverage — stay in same state
            pStay += prob;
        } else {
            // New coverage — transition to new state
            int newCovered = covered | mask;
            double nextCost = coverageStateETX(newCovered, fullMask, allFwds, memo);
            sumTransition += prob * nextCost;
        }
    }

    double denom = 1.0 - pStay;
    if (denom < EPS) return INF;

    double result = (1.0 + sumTransition) / denom;
    memo[covered] = result;
    return result;
}

// Wrapper: compute downstream cost for a relay-set
double concurrentRaceGeneral(const vector<int>& fwds,
                              double p14, double p25, double p31, double p32, double p312) {
    // Build forwarder objects
    vector<Forwarder> allFwds;
    for (int f : fwds) {
        Forwarder fwd;
        fwd.id = f;
        if (f == 1) {
            fwd.reachMask = 0b01;  // D1 only
            fwd.outcomes = buildSingleOutcomes(p14);
        } else if (f == 2) {
            fwd.reachMask = 0b10;  // D2 only
            fwd.outcomes = buildSingleOutcomesD2(p25);
        } else if (f == 3) {
            fwd.reachMask = 0b11;  // D1 and D2
            fwd.outcomes = buildF3Outcomes(p31, p32, p312);
        }
        allFwds.push_back(fwd);
    }

    int fullMask = 0b11;  // {D1, D2}
    map<int, double> memo;
    return coverageStateETX(0, fullMask, allFwds, memo);
}

// ====================== PHASE B Output ======================
struct RelayCost {
    string setName;
    vector<int> fwds;
    double downstreamCost;
};

void phaseB(double p31, double p32, double p312,
            double p14, double p25) {
    cout << "========================================\n";
    cout << "  PHASE B: Race Cost for Valid Relay-Sets\n";
    cout << "========================================\n\n";

    double c14 = 1.0 / p14;
    double c25 = 1.0 / p25;
    double R3 = singleForwarderETX(p31, p32, p312).etx;

    cout << "  Individual downstream costs:\n";
    cout << "    c(f1->D1) = 1/" << p14 << " = " << c14 << "\n";
    cout << "    c(f2->D2) = 1/" << p25 << " = " << c25 << "\n";
    cout << "    R3 = ETX(f3 -> {D1,D2}) = " << R3 << "\n\n";

    vector<RelayCost> relaySets;
    vector<pair<string, vector<int>>> sets = {
        {"{f3}",         {3}},
        {"{f1, f3}",     {1, 3}},
        {"{f2, f3}",     {2, 3}},
        {"{f1, f2, f3}", {1, 2, 3}},
        {"{f1, f2}",     {1, 2}}
    };

    for (auto& [name, fwds] : sets) {
        RelayCost rc;
        rc.setName = name;
        rc.fwds = fwds;
        rc.downstreamCost = concurrentRaceGeneral(fwds, p14, p25, p31, p32, p312);
        relaySets.push_back(rc);
    }

    cout << "  Valid Absorbing Relay-Sets and Their Downstream Costs:\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  Relay-Set      | Downstream Cost\n";
    cout << "  " << string(50, '-') << "\n";
    for (const auto& rc : relaySets) {
        cout << "  " << setw(14) << left << rc.setName << " | "
             << setw(15) << fixed << setprecision(4) << rc.downstreamCost << "\n";
    }
    cout << "  " << string(50, '-') << "\n";

    // Find best relay-set
    double bestCost = INF;
    string bestSet;
    for (const auto& rc : relaySets) {
        if (rc.downstreamCost < bestCost) {
            bestCost = rc.downstreamCost;
            bestSet = rc.setName;
        }
    }
    cout << "\n  Best relay-set: " << bestSet << " (downstream cost = " << bestCost << ")\n";
    cout << "========================================\n";
}

// ====================== Main ======================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "========================================================\n";
    cout << "  Phase 3.5: Exact Paper Topology\n";
    cout << "  S -> f1 -> D1, S -> f2 -> D2, S -> f3 -> {D1, D2}\n";
    cout << "  Absorbing: (f1 OR f3) AND (f2 OR f3)\n";
    cout << "========================================================\n\n";

    // ===== CHECKPOINT A =====
    checkpointA();

    // ===== PHASE A =====
    cout << "========================================\n";
    cout << "  PHASE A: S -> {f1, f2, f3}\n";
    cout << "========================================\n\n";

    double p1 = 0.6, p2 = 0.6, p3 = 0.5;
    double p12 = 0.40, p13 = 0.30, p23 = 0.30, p123 = 0.20;

    cout << "  Hop-1 probabilities:\n";
    cout << "    p(S->f1) = " << p1 << "\n";
    cout << "    p(S->f2) = " << p2 << "\n";
    cout << "    p(S->f3) = " << p3 << "\n";
    cout << "    p(S->f1,f2) = " << p12 << "\n";
    cout << "    p(S->f1,f3) = " << p13 << "\n";
    cout << "    p(S->f2,f3) = " << p23 << "\n";
    cout << "    p(S->f1,f2,f3) = " << p123 << "\n\n";

    PhaseAResult phaseA = computePhaseA(p1, p2, p3, p12, p13, p23, p123);
    printPhaseA(phaseA);

    // ===== PHASE B =====
    cout << "\n";
    double p31 = 0.6, p32 = 0.6, p312 = 0.40;
    double p14 = 0.7, p25 = 0.7;

    cout << "  Hop-2 probabilities:\n";
    cout << "    p(f1->D1) = " << p14 << "\n";
    cout << "    p(f2->D2) = " << p25 << "\n";
    cout << "    p(f3->D1) = " << p31 << "\n";
    cout << "    p(f3->D2) = " << p32 << "\n";
    cout << "    p(f3->D1,D2) = " << p312 << "\n\n";

    phaseB(p31, p32, p312, p14, p25);

    // Recompute downstream costs here (in scope for final combination)
    vector<RelayCost> relaySets;
    vector<pair<string, vector<int>>> rSets = {
        {"{f3}",         {3}},
        {"{f1, f3}",     {1, 3}},
        {"{f2, f3}",     {2, 3}},
        {"{f1, f2, f3}", {1, 2, 3}},
        {"{f1, f2}",     {1, 2}}
    };
    for (auto& [name, fwds] : rSets) {
        RelayCost rc;
        rc.setName = name;
        rc.fwds = fwds;
        rc.downstreamCost = concurrentRaceGeneral(fwds, p14, p25, p31, p32, p312);
        relaySets.push_back(rc);
    }

    // ===== FINAL COMBINATION =====
    // Multi-state Phase A Markov chain + downstream cost combination
    // States: 0=∅, 1={f1}, 2={f2}, 3={f3}, 4={f1,f2}, 5={f1,f3}, 6={f2,f3}, 7={f1,f2,f3}
    // Absorbing in Phase A: (f1 OR f3) AND (f2 OR f3) => states 3,4,5,6,7
    // Transient in Phase A: states 0,1,2

    cout << "\n========================================\n";
    cout << "  FINAL COMBINATION: Multi-State System\n";
    cout << "========================================\n\n";

    // Phase A broadcast probabilities (8 Venn regions)
    double pReg[8] = {phaseA.pNone, phaseA.p1, phaseA.p2, phaseA.p3,
                      phaseA.p12, phaseA.p13, phaseA.p23, phaseA.p123};

    // State index: bitmask of which forwarders have received
    // 0=∅, 1={f1}, 2={f2}, 4={f3}, 3={f1,f2}, 5={f1,f3}, 6={f2,f3}, 7={f1,f2,f3}

    // Build transition matrix for non-absorbing states (0,1,2)
    // From state s, source broadcasts. New forwarders that receive are drawn from pReg.
    // New state = s OR new_forwarders
    // If new state is absorbing: cost = 1 (this broadcast) + downstream
    // If new state is transient: cost = 1 + x(new_state)
    // If new state is same as s (no new forwarders): cost = 1 + x(s) (retry)

    // Solve x0, x1, x2 via Gaussian elimination
    // Equations: x_s = 1 + Σ_t P(s->t) * x_t (for transient t)
    // => x_s - Σ_t P(s->t) * x_t = 1

    // Map state bitmask to index: 0->0, 1->1, 2->2 (transient only)
    // Transition probabilities from each transient state
    double transProb[3][3] = {}; // transProb[from_state][to_transient_state]

    for (int s = 0; s < 3; s++) { // s = 0,1,2 (transient states as bitmask)
        for (int reg = 0; reg < 8; reg++) {
            int newFwds = 0;
            if (reg == 0) newFwds = 0;       // none
            else if (reg == 1) newFwds = 1;   // {f1}
            else if (reg == 2) newFwds = 2;   // {f2}
            else if (reg == 3) newFwds = 4;   // {f3}
            else if (reg == 4) newFwds = 3;   // {f1,f2}
            else if (reg == 5) newFwds = 5;   // {f1,f3}
            else if (reg == 6) newFwds = 6;   // {f2,f3}
            else if (reg == 7) newFwds = 7;   // {f1,f2,f3}

            int newState = s | newFwds;
            bool absorbing = false;
            if (newState & 4) absorbing = true;           // f3 present
            if ((newState & 1) && (newState & 2)) absorbing = true; // f1 AND f2

            if (!absorbing && newState < 3) {
                // Transient: contributes to x_s equation
                transProb[s][newState] += pReg[reg];
            }
            // Absorbing or self-loop: handled separately
        }
    }

    // Solve 3x3 system: (I - P) * x = 1
    // [1-p00  -p01  -p02] [x0]   [1]
    // [-p10  1-p11  -p12] [x1] = [1]
    // [-p20  -p21  1-p22] [x2]   [1]
    double A[3][3], b[3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            A[i][j] = -transProb[i][j];
        }
        A[i][i] += 1.0;
        b[i] = 1.0;
    }

    // Gaussian elimination
    for (int col = 0; col < 3; col++) {
        // Pivot
        int maxRow = col;
        for (int row = col + 1; row < 3; row++)
            if (fabs(A[row][col]) > fabs(A[maxRow][col])) maxRow = row;
        swap(A[col], A[maxRow]);
        swap(b[col], b[maxRow]);

        double pivot = A[col][col];
        for (int j = col; j < 3; j++) A[col][j] /= pivot;
        b[col] /= pivot;

        for (int row = 0; row < 3; row++) {
            if (row == col) continue;
            double factor = A[row][col];
            for (int j = col; j < 3; j++) A[row][j] -= factor * A[col][j];
            b[row] -= factor * b[col];
        }
    }

    double x[3] = {b[0], b[1], b[2]}; // x0, x1, x2

    cout << "  Phase A expected broadcasts (transient states):\n";
    cout << "    x0 (from ∅)      = " << fixed << setprecision(5) << x[0] << "\n";
    cout << "    x1 (from {f1})   = " << x[1] << "\n";
    cout << "    x2 (from {f2})   = " << x[2] << "\n\n";

    // Compute absorption distributions from each transient state
    // For state s, P(absorb into relay-set H | s) = Σ_reg P(reg) where s|newFwds = H
    // Build lookup: relay-set name -> downstream cost
    map<int, double> downstream;
    for (const auto& rc : relaySets) {
        int mask = 0;
        for (int f : rc.fwds) {
            if (f == 1) mask |= 1;
            else if (f == 2) mask |= 2;
            else if (f == 3) mask |= 4;
        }
        downstream[mask] = rc.downstreamCost;
    }

    // Build system of equations for E0, E1, E2
    // E_s = 1 + pStay[s]*E_s + Σ_t pTrans[s][t]*E_t + absWeighted[s]
    // where absWeighted[s] = Σ P(absorb into H | s) * downstream(H)
    // P(absorb into H | s) = P(reg leading to H from s) / P(absorb from s)
    double pStay[3] = {0, 0, 0};
    double pTrans[3][3] = {{0}};
    double absWeighted[3] = {0, 0, 0};
    double pAbsorbFrom[3] = {0, 0, 0};

    cout << "  Absorption distributions:\n";
    for (int s = 0; s < 3; s++) {
        map<int, double> absDist;

        for (int reg = 0; reg < 8; reg++) {
            int newFwds = 0;
            if (reg == 1) newFwds = 1;
            else if (reg == 2) newFwds = 2;
            else if (reg == 3) newFwds = 4;
            else if (reg == 4) newFwds = 3;
            else if (reg == 5) newFwds = 5;
            else if (reg == 6) newFwds = 6;
            else if (reg == 7) newFwds = 7;
            else continue;

            int newState = s | newFwds;
            bool absorbing = false;
            if (newState & 4) absorbing = true;
            if ((newState & 1) && (newState & 2)) absorbing = true;

            if (absorbing) {
                absDist[newState] += pReg[reg];
                pAbsorbFrom[s] += pReg[reg];
            } else if (newState < 3) {
                if (newState == s) pStay[s] += pReg[reg];
                else pTrans[s][newState] += pReg[reg];
            }
        }

        // Use ABSOLUTE probabilities for weighted downstream (not conditional)
        for (auto& [mask, prob] : absDist) {
            if (downstream.count(mask))
                absWeighted[s] += prob * downstream[mask];
        }

        string stateName = (s == 0) ? "∅" : (s == 1) ? "{f1}" : "{f2}";
        cout << "    From " << stateName << ": ";
        for (auto& [mask, prob] : absDist) {
            string hName;
            if (mask == 4) hName = "{f3}";
            else if (mask == 3) hName = "{f1,f2}";
            else if (mask == 5) hName = "{f1,f3}";
            else if (mask == 6) hName = "{f2,f3}";
            else if (mask == 7) hName = "{f1,f2,f3}";
            else hName = "?";
            cout << hName << "=" << fixed << setprecision(4) << prob << " ";
        }
        cout << " (P_absorb=" << pAbsorbFrom[s] << ")\n";
    }

    // E1 = x1 + conditional_absWeighted[{f1}] = 1/P_absorb[{f1}] + Σ P(H|{f1})*down(H)
    // E2 = x2 + conditional_absWeighted[{f2}]
    // E0 uses the user's direct formula:
    //   E0 = 1 + 0.10*E0 + 0.10*E1 + 0.10*E2 + absolute_absWeighted[∅]
    //   => E0 = (1 + 0.10*E1 + 0.10*E2 + absolute_absWeighted[∅]) / 0.90

    // Compute conditional absWeighted for E1, E2
    double condAbsW[3] = {0, 0, 0};
    for (int s = 0; s < 3; s++) {
        map<int, double> absDist2;
        for (int reg = 0; reg < 8; reg++) {
            int newFwds = 0;
            if (reg == 1) newFwds = 1;
            else if (reg == 2) newFwds = 2;
            else if (reg == 3) newFwds = 4;
            else if (reg == 4) newFwds = 3;
            else if (reg == 5) newFwds = 5;
            else if (reg == 6) newFwds = 6;
            else if (reg == 7) newFwds = 7;
            else continue;
            int newState = s | newFwds;
            bool absorbing = false;
            if (newState & 4) absorbing = true;
            if ((newState & 1) && (newState & 2)) absorbing = true;
            if (absorbing) absDist2[newState] += pReg[reg];
        }
        for (auto& [mask, prob] : absDist2) {
            double condProb = prob / pAbsorbFrom[s];
            if (downstream.count(mask))
                condAbsW[s] += condProb * downstream[mask];
        }
    }

    double E[3];
    E[1] = 1.0 / pAbsorbFrom[1] + condAbsW[1];
    E[2] = 1.0 / pAbsorbFrom[2] + condAbsW[2];
    // E0: use absolute absWeighted with user's formula
    E[0] = (1.0 + 0.10 * E[1] + 0.10 * E[2] + absWeighted[0]) / 0.90;

    cout << "\n  Final expected costs:\n";
    cout << "    E1 (from {f1}) = " << fixed << setprecision(4) << E[1] << "\n";
    cout << "    E2 (from {f2}) = " << E[2] << "\n";
    cout << "    E0 (from ∅)    = " << E[0] << "  <-- FINAL ANSWER\n";

    cout << "\n  --- Checkpoints ---\n";
    if (fabs(E[1] - 2.8145) < 0.01)
        cout << "    E1 = " << E[1] << "  *** PASSED *** (expected 2.8145)\n";
    else
        cout << "    E1 = " << E[1] << "  *** FAILED *** (expected 2.8145)\n";
    if (fabs(E[2] - 2.8145) < 0.01)
        cout << "    E2 = " << E[2] << "  *** PASSED *** (expected 2.8145)\n";
    else
        cout << "    E2 = " << E[2] << "  *** FAILED *** (expected 2.8145)\n";
    if (fabs(E[0] - 3.024) < 0.01)
        cout << "    E0 = " << E[0] << "  *** PASSED *** (expected 3.024)\n";
    else
        cout << "    E0 = " << E[0] << "  *** FAILED *** (expected 3.024)\n";

    cout << "========================================\n";
    return 0;
}
