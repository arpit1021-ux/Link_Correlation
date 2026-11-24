#include <bits/stdc++.h>
using namespace std;

const double INF = 1e12;
const double EPS = 1e-12;
mt19937 rng(42);
uniform_real_distribution<double> U(0.0, 1.0);

struct ProbStore {
    map<int, map<int, double>> marg;
    map<int, map<vector<int>, double>> joint;
};

double get_joint(int src, vector<int> tgt, const ProbStore& ps) {
    if (tgt.empty()) return 1.0;
    sort(tgt.begin(), tgt.end());
    if (tgt.size() == 1) {
        if (ps.marg.count(src) && ps.marg.at(src).count(tgt[0]))
            return ps.marg.at(src).at(tgt[0]);
        return 0.0;
    }
    auto it = ps.joint.find(src);
    if (it != ps.joint.end()) {
        auto it2 = it->second.find(tgt);
        if (it2 != it->second.end()) return it2->second;
    }
    double p = 1.0;
    for (int t : tgt)
        p *= ps.marg.count(src) && ps.marg.at(src).count(t) ? ps.marg.at(src).at(t) : 0.0;
    return p;
}

double prob_exactly(int src, const vector<int>& all_tgt, const vector<int>& recv, const ProbStore& ps) {
    int n = all_tgt.size(), recv_mask = 0;
    for (int r : recv) for (int i = 0; i < n; i++) if (all_tgt[i] == r) recv_mask |= (1 << i);
    double p = 0.0;
    for (int sup = recv_mask; sup < (1 << n); sup = (sup + 1) | recv_mask) {
        vector<int> st;
        for (int i = 0; i < n; i++) if (sup & (1 << i)) st.push_back(all_tgt[i]);
        int extra = (int)st.size() - (int)recv.size();
        p += (extra % 2 == 0 ? 1 : -1) * get_joint(src, st, ps);
    }
    return max(0.0, p);
}

double cEAX(int src, const vector<int>& tgt, const ProbStore& ps) {
    int n = tgt.size();
    if (n == 0) return 0.0;
    if (n == 1) {
        if (ps.marg.count(src) && ps.marg.at(src).count(tgt[0])) {
            double p = ps.marg.at(src).at(tgt[0]);
            return (p > EPS) ? 1.0 / p : INF;
        }
        return INF;
    }
    double num = 1.0, pnone = 1.0;
    for (int mask = 1; mask < (1 << n); mask++) {
        vector<int> recv, rem;
        for (int i = 0; i < n; i++) (mask & (1 << i) ? recv : rem).push_back(tgt[i]);
        double pe = prob_exactly(src, tgt, recv, ps);
        pnone -= pe;
        if (!rem.empty()) num += pe * cEAX(src, rem, ps);
    }
    if (pnone < -EPS) pnone = 0.0;
    return (1.0 - pnone > EPS) ? num / (1.0 - pnone) : INF;
}

vector<int> set_to_vec(const set<int>& s) { return vector<int>(s.begin(), s.end()); }
void pset(const vector<int>& s) { cout << "{"; for (int i=0;i<(int)s.size();i++){if(i)cout<<",";cout<<s[i];} cout << "}"; }

struct Region { double upper; vector<int> recv; };

vector<Region> build_partition(int src, const vector<int>& tgt, const ProbStore& ps) {
    int n = tgt.size();
    vector<Region> regs;
    double cum = 0.0;
    double pnone = 1.0;
    for (int mask = 1; mask < (1 << n); mask++) {
        vector<int> recv;
        for (int i = 0; i < n; i++) if (mask & (1 << i)) recv.push_back(tgt[i]);
        pnone -= prob_exactly(src, tgt, recv, ps);
    }
    pnone = max(0.0, pnone);
    cum += pnone;
    regs.push_back({cum, {}});
    for (int mask = 1; mask < (1 << n); mask++) {
        vector<int> recv;
        for (int i = 0; i < n; i++) if (mask & (1 << i)) recv.push_back(tgt[i]);
        double p = max(0.0, prob_exactly(src, tgt, recv, ps));
        cum += p;
        regs.push_back({cum, recv});
    }
    if (!regs.empty()) regs.back().upper = 1.0;
    return regs;
}

vector<int> draw_outcome(double r, const vector<Region>& regs) {
    for (const auto& reg : regs) if (r < reg.upper) return reg.recv;
    return regs.back().recv;
}

set<int> get_downstream(int f, const set<int>& ds, const map<int,set<int>>& adj) {
    set<int> r;
    if (adj.count(f)) for (int d : adj.at(f)) if (ds.count(d)) r.insert(d);
    return r;
}

/* Paper's Eq.(1): cEAX_{S,DS}(f_s)
   For each subset fj of f_s that receives:
     DSF_P = destinations reachable from fj (covered cooperatively)
     DSF_U = destinations NOT reachable (source retransmits)
   Cost for DSF_P = cooperative hop-2 cost (teacher's min formula)
   Cost for DSF_U = recursive call with source retransmitting */
map<string,double> memo;
string mkey(int s, vector<int> d, vector<int> f) {
    string k = to_string(s)+"|";
    for(int x:d) k+=to_string(x)+","; k+="|";
    for(int x:f) k+=to_string(x)+","; return k;
}

/* Compute cooperative cost for forwarder set fj to cover dest set dsf_p */
double coop_cost(const vector<int>& fj, const set<int>& dsf_p,
                 const map<int,set<int>>& adj, const ProbStore& ps) {
    if (dsf_p.empty()) return 0.0;
    if (fj.size() == 1) {
        set<int> ds = get_downstream(fj[0], dsf_p, adj);
        return cEAX(fj[0], set_to_vec(ds), ps);
    }
    /* For 2+ forwarders: enumerate all splits, pick minimum total cost */
    int n = fj.size();
    double best = INF;
    for (int mask = 1; mask < (1 << n); mask++) {
        vector<int> coverers;
        for (int i = 0; i < n; i++) if (mask & (1 << i)) coverers.push_back(fj[i]);
        set<int> covered;
        for (int c : coverers) for (int d : dsf_p)
            if (adj.count(c) && adj.at(c).count(d)) covered.insert(d);
        if (covered != dsf_p) continue;
        double cost = 0.0;
        set<int> remaining = dsf_p;
        for (int c : coverers) {
            set<int> my_ds;
            for (int d : remaining) if (adj.count(c) && adj.at(c).count(d)) my_ds.insert(d);
            if (!my_ds.empty()) cost += cEAX(c, set_to_vec(my_ds), ps);
            for (int d : my_ds) remaining.erase(d);
        }
        best = min(best, cost);
    }
    return best;
}

double paper_cEAX(int src, vector<int> ds, vector<int> fs,
                   const map<int,set<int>>& adj, const ProbStore& ps) {
    if (ds.empty()) return 0.0;
    if (fs.empty()) return INF;
    string key = mkey(src, ds, fs);
    if (memo.count(key)) return memo[key];

    vector<int> active;
    for (int f : fs) for (int d : ds)
        if (adj.count(f) && adj.at(f).count(d)) { active.push_back(f); break; }
    if (active.empty()) return memo[key] = cEAX(src, ds, ps);

    int n = active.size();
    double num = 1.0, pnone = 1.0;
    for (int mask = 1; mask < (1 << n); mask++) {
        vector<int> fj;
        for (int i = 0; i < n; i++) if (mask & (1 << i)) fj.push_back(active[i]);
        double beta = prob_exactly(src, active, fj, ps);
        pnone -= beta;

        set<int> dsf_p, dsf_u;
        for (int d : ds) {
            bool ok = false;
            for (int f : fj) if (adj.count(f) && adj.at(f).count(d)) { ok = true; break; }
            (ok ? dsf_p : dsf_u).insert(d);
        }

        double cost_fj = coop_cost(fj, dsf_p, adj, ps);
        if (!dsf_u.empty())
            cost_fj += paper_cEAX(src, set_to_vec(dsf_u), active, adj, ps);
        num += beta * cost_fj;
    }
    if (pnone < -EPS) pnone = 0.0;
    double denom = 1.0 - pnone;
    return memo[key] = (denom > EPS) ? num / denom : INF;
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    cout << fixed << setprecision(6);

    cout << "====================================================\n";
    cout << "  Case Study: 0 -> {1,2,3} -> {4,5,6}\n";
    cout << "  M-LCAR paper (Rathore, Dhaka, Bose) implementation\n";
    cout << "====================================================\n\n";

    double p01,p02,p03,p12,p13,p23,p123;
    cout << "Hop-1: p(0->1) p(0->2) p(0->3) p(01) p(02) p(03) p(0123):\n  ";
    cin >> p01 >> p02 >> p03 >> p12 >> p13 >> p23 >> p123;

    double p14,p15,p145;
    cout << "Hop-2 node 1: p(1->4) p(1->5) p(145):\n  ";
    cin >> p14 >> p15 >> p145;

    double p25,p26,p256;
    cout << "Hop-2 node 2: p(2->5) p(2->6) p(256):\n  ";
    cin >> p25 >> p26 >> p256;

    double p35,p36,p356;
    cout << "Hop-2 node 3: p(3->5) p(3->6) p(356):\n  ";
    cin >> p35 >> p36 >> p356;

    int N; cout << "N: "; cin >> N;

    ProbStore prob;
    prob.marg[0][1]=p01; prob.marg[0][2]=p02; prob.marg[0][3]=p03;
    prob.joint[0][{1,2}]=p12; prob.joint[0][{1,3}]=p13;
    prob.joint[0][{2,3}]=p23; prob.joint[0][{1,2,3}]=p123;
    prob.marg[1][4]=p14; prob.marg[1][5]=p15; prob.joint[1][{4,5}]=p145;
    prob.marg[2][5]=p25; prob.marg[2][6]=p26; prob.joint[2][{5,6}]=p256;
    prob.marg[3][5]=p35; prob.marg[3][6]=p36; prob.joint[3][{5,6}]=p356;

    map<int,set<int>> adj = {{0,{1,2,3}}, {1,{4,5}}, {2,{5,6}}, {3,{5,6}}};
    set<int> DS = {4,5,6};
    vector<int> cands = {1,2,3};

    vector<vector<int>> valid;
    for (int m=1;m<8;m++) {
        vector<int> f; for(int i=0;i<3;i++) if(m&(1<<i)) f.push_back(cands[i]);
        set<int> cov; for(int ff:f) for(int d:adj[ff]) if(DS.count(d)) cov.insert(d);
        if(cov==DS) valid.push_back(f);
    }

    /* Per-forwarder costs */
    map<int,double> R;
    for (int f : cands) R[f] = cEAX(f, set_to_vec(get_downstream(f,DS,adj)), prob);

    cout << "\n--- Per-forwarder costs ---\n";
    for (int f:cands) { cout<<"  R"<<f<<" = "<<R[f]<<"\n"; }

    /* Teacher's formula */
    map<vector<int>,double> T;
    T[{1,2}]=min(R[1]+1/prob.marg[2][6], R[2]+1/prob.marg[1][4]);
    T[{1,3}]=min(R[1]+1/prob.marg[3][6], R[3]+1/prob.marg[1][4]);
    T[{2,3}]=min(R[2],R[3]);
    T[{1,2,3}]=R[1]+min(R[2],R[3]);

    cout << "\n--- Teacher's formula (downstream cooperative) ---\n";
    cout<<"  R12="<<T[{1,2}]<<"  R13="<<T[{1,3}]<<"  R23="<<T[{2,3}]<<"  R123="<<T[{1,2,3}]<<"\n";

    /* Paper's Eq.(1) */
    cout << "\n--- Paper's Eq.(1) (full recursive) ---\n";
    map<vector<int>,double> P;
    for (auto& fs : valid) { memo.clear(); P[fs]=paper_cEAX(0,set_to_vec(DS),fs,adj,prob); }
    for (auto& fs : valid) { cout<<"  cEAX(DS,";pset(fs);cout<<") = "<<P[fs]<<"\n"; }

    /* Summary per set */
    cout << "\n--- Comparison per forwarder set ---\n";
    for (auto& fs : valid) {
        double h1=cEAX(0,fs,prob);
        cout<<"  ";pset(fs);
        cout<<" : Teacher(hop2only)="<<T[fs]
            <<"  Teacher+hop1="<<h1+T[fs]
            <<"  Paper="<<P[fs]<<"\n";
    }

    /* Simulation */
    cout << "\n--- Simulation (N="<<N<<") ---\n";
    auto h1p = build_partition(0, cands, prob);

    for (auto& fs : valid) {
        long long tx=0;
        for (int pkt=0;pkt<N;pkt++) {
            set<int> ds_rem=DS;
            while (!ds_rem.empty()) {
                set<int> got;
                while(got.empty()){
                    auto o=draw_outcome(U(rng),h1p);
                    for(int f:o) if(find(fs.begin(),fs.end(),f)!=fs.end()) got.insert(f);
                    tx++;
                }
                bool prog=true;
                while(prog){
                    prog=false;
                    for(int f:fs){
                        if(!got.count(f))continue;
                        set<int> reach;
                        for(int d:ds_rem) if(adj.count(f)&&adj.at(f).count(d)) reach.insert(d);
                        if(reach.empty())continue;
                        auto parts=build_partition(f,set_to_vec(reach),prob);
                        auto o=draw_outcome(U(rng),parts);
                        tx++;
                        for(int d:o) if(ds_rem.count(d)){ds_rem.erase(d);prog=true;}
                    }
                }
            }
        }
        double sim=(double)tx/N;
        double h1=cEAX(0,fs,prob);
        double ep=fabs(sim-P[fs])/P[fs]*100;
        double et=fabs(sim-(h1+T[fs]))/(h1+T[fs])*100;
        cout<<"  ";pset(fs);
        cout<<" : Sim="<<sim<<"  (Paper err="<<ep<<"%, Teacher+hop1 err="<<et<<"%)  \n";
    }

    return 0;
}
