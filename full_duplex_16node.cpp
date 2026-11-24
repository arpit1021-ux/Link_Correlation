// Network-wide FORCED full duplex: every node holding the packet transmits every slot
// for as long as it is still needed (rule S): v is needed iff some uncovered destination d
// is reachable from v and no out-neighbour of v that already holds the packet can reach d.
// Generalises the validated Phase 1-5 full duplex (source stops once its informed set is
// sufficient; forwarders then run concurrently) to every layer. Exact DP + Monte Carlo.
#include <bits/stdc++.h>
using namespace std;
struct E{int to;double cost,p;};
int main(){
  int N,S,D; cin>>N>>S>>D; vector<int> dl(D); for(auto&d:dl)cin>>d;
  map<int,int> db; for(int i=0;i<D;i++) db[dl[i]]=1<<i;
  vector<vector<E>> adj(N);
  for(int i=0;i<N;i++){int k;cin>>k;adj[i].resize(k);for(auto&e:adj[i]){cin>>e.to>>e.cost;e.p=1/e.cost;}}
  vector<map<vector<int>,double>> corr(N);
  for(int i=0;i<N;i++){ if(adj[i].size()<2)continue; int C;cin>>C; while(C--){int m;cin>>m;vector<int> ids(m);for(auto&x:ids)cin>>x;double v;cin>>v;sort(ids.begin(),ids.end());corr[i][ids]=v;}}
  long long T;cin>>T;
  // per-node outcome distribution over neighbour node-bitmasks (Mobius inversion)
  vector<vector<pair<uint32_t,double>>> out(N);
  for(int v=0;v<N;v++){int k=adj[v].size(); if(!k)continue;
    vector<int> par(k);iota(par.begin(),par.end(),0);
    function<int(int)> f=[&](int x){return par[x]==x?x:par[x]=f(par[x]);};
    auto idx=[&](int nb){for(int i=0;i<k;i++)if(adj[v][i].to==nb)return i;return -1;};
    for(auto&c:corr[v]) if(c.first.size()>=2) for(size_t i=1;i<c.first.size();i++) par[f(idx(c.first[0]))]=f(idx(c.first[i]));
    auto pal=[&](int mask)->double{ if(!mask)return 1; map<int,vector<int>> bc; for(int i=0;i<k;i++)if(mask>>i&1)bc[f(i)].push_back(i);
      double r=1; for(auto&cp:bc){auto&li=cp.second; if(li.size()==1){r*=adj[v][li[0]].p;continue;} vector<int> ids;for(int l:li)ids.push_back(adj[v][l].to);sort(ids.begin(),ids.end());
        if(corr[v].count(ids)){r*=corr[v][ids];continue;} for(int l:li)r*=adj[v][l].p;} return r;};
    for(int Tm=0;Tm<(1<<k);Tm++){double a=0;int rest=((1<<k)-1)&~Tm;for(int R=rest;;R=(R-1)&rest){a+=((__builtin_popcount(R)&1)?-1:1)*pal(Tm|R);if(!R)break;}
      if(a>1e-12){uint32_t nb=0;for(int i=0;i<k;i++)if(Tm>>i&1)nb|=1u<<adj[v][i].to; out[v].push_back({nb,a});}}
  }
  // destination reachability
  vector<int> reach(N,0); for(int u=0;u<N;u++) if(db.count(u)) reach[u]=db[u];
  for(bool ch=true;ch;){ch=false;for(int u=0;u<N;u++)for(auto&e:adj[u]){int n=reach[u]|reach[e.to];if(n!=reach[u]){reach[u]=n;ch=true;}}}
  uint32_t destMask=0; for(int d:dl) destMask|=1u<<d;
  auto active=[&](uint32_t s){ vector<int> A; int unc=0; for(int i=0;i<D;i++) if(!(s>>dl[i]&1)) unc|=1<<i;
    for(int v=0;v<N;v++){ if(!(s>>v&1)||adj[v].empty())continue; int need=reach[v]&unc;
      for(auto&e:adj[v]) if(s>>e.to&1) need&=~reach[e.to];
      if(need)A.push_back(v);} return A;};
  // exact DP
  unordered_map<uint32_t,pair<double,double>> memo;
  const double INF=1e18;
  function<pair<double,double>(uint32_t)> solve=[&](uint32_t s)->pair<double,double>{
    if((s&destMask)==destMask) return {0,0};
    auto it=memo.find(s); if(it!=memo.end()) return it->second;
    auto A=active(s); if(A.empty()) return memo[s]={INF,INF};
    map<uint32_t,double> dist; dist[0]=1;
    for(int v:A){ map<uint32_t,double> nd; for(auto&kv:dist) for(auto&o:out[v]) nd[kv.first|(o.first&~s)]+=kv.second*o.second; dist=nd; }
    double pst=dist.count(0)?dist[0]:0; double ns=1,nr=A.size();
    for(auto&kv:dist){ if(!kv.first)continue; auto c=solve(s|kv.first); ns+=kv.second*c.first; nr+=kv.second*c.second; }
    return memo[s]={ns/(1-pst),nr/(1-pst)};
  };
  auto r=solve(1u<<S);
  // Monte Carlo with the same rule
  mt19937_64 rng(777); uniform_real_distribution<double> U(0,1);
  double sl=0,rs=0;
  for(long long t=0;t<T;t++){ uint32_t s=1u<<S; long long slots=0,res=0;
    while((s&destMask)!=destMask){ auto A=active(s); if(A.empty())break; slots++; res+=A.size(); uint32_t nw=0;
      for(int v:A){ double x=U(rng),c=0; for(auto&o:out[v]){c+=o.second;if(x<=c){nw|=o.first;break;}} } s|=nw; }
    sl+=slots; rs+=res; }
  printf("states=%zu\nanalytical : slots=%.4f resource=%.4f\nsimulated  : slots=%.4f resource=%.4f\n",memo.size(),r.first,r.second,sl/T,rs/T);
}