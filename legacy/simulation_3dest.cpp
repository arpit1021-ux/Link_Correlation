#include <bits/stdc++.h>
#include <random>
using namespace std;

/* ================= RANDOM ================= */
mt19937 rng(time(NULL));
uniform_real_distribution<double> dist(0.0, 1.0);
double urand() { return dist(rng); }

/* ================= ETX2 ================= */
double ETX2(double pA, double pB, double pAB) {
    double none = 1 - pA - pB + pAB;
    double onlyA = pA - pAB;
    double onlyB = pB - pAB;
    double both = pAB;

    return (none
          + onlyA * (1 + 1 / pB)
          + onlyB * (1 + 1 / pA)
          + both) / (1 - none);
}

/* ================= PACKET ================= */
struct Packet {
    bool at1=false, at2=false, at3=false;
    bool d4=false, d5=false, d6=false;
};

int main(){

    cout<<fixed<<setprecision(6);

    /* ================= INPUT ================= */
    double p01,p02,p03,p012,p013,p023,p0123;
    double p14,p15,p145;
    double p25,p26,p256;
    double p35,p36,p356;
    int N;

    cin >> p01 >> p02 >> p03 >> p012 >> p013 >> p023 >> p0123;
    cin >> p14 >> p15 >> p145;
    cin >> p25 >> p26 >> p256;
    cin >> p35 >> p36 >> p356;
    cin >> N;

    /* ================= ANALYTICAL COSTS ================= */

    double R1 = ETX2(p14,p15,p145);
    double R2 = ETX2(p25,p26,p256);
    double R3 = ETX2(p35,p36,p356);

    // Multi-channel combinations
    double R12 = min(R1 + 1/p26, R2 + 1/p14);
    double R13 = min(R1 + 1/p36, R3 + 1/p14);
    double R23 = min(R2, R3);
    double R123 = R1 + R23;

    cout<<"R1="<<R1<<"\n";
    cout<<"R2="<<R2<<"\n";
    cout<<"R3="<<R3<<"\n";
    cout<<"R12="<<R12<<"\n";
    cout<<"R13="<<R13<<"\n";
    cout<<"R23="<<R23<<"\n";
    cout<<"R123="<<R123<<"\n";

    /* ================= HEURISTIC ================= */

    vector<pair<string,double>> sets = {
        {"12",R12},
        {"13",R13},
        {"123",R123}
    };

    sort(sets.begin(),sets.end(),
         [](auto &a,auto &b){ return a.second<b.second; });

    string best = sets[0].first;

    cout<<"Best Forwarder Set: "<<best<<"\n";

    // FINAL analytical (aligned with simulation)
    double analytical = sets[0].second;

    cout<<"Analytical ETX (Final) = "<<analytical<<"\n";

    /* ================= SIMULATION ================= */

    int delivered = 0;
    long long transmissions = 0;

    double p_all = p0123;
    double p_12  = p012 - p_all;
    double p_13  = p013 - p_all;
    double p_23  = p023 - p_all;
    double p_1   = p01 - p012 - p013 + p_all;
    double p_2   = p02 - p012 - p023 + p_all;
    double p_3   = p03 - p013 - p023 + p_all;
    double p_none = 1.0 - (p_1 + p_2 + p_3 + p_12 + p_13 + p_23 + p_all);

    double none14 = 1 - p14 - p15 + p145;
    double only4  = p14 - p145;
    double only5_1 = p15 - p145;

    double none25 = 1 - p25 - p26 + p256;
    double only5_2 = p25 - p256;
    double only6_2 = p26 - p256;

    double none35 = 1 - p35 - p36 + p356;
    double only5_3 = p35 - p356;
    double only6_3 = p36 - p356;

    while(delivered < N){

        Packet pkt;

        /* ===== STAGE 1 (UNCHANGED) ===== */
        while(!(pkt.at1 && (pkt.at2 || pkt.at3))){
            double r = urand();

            if(r < p_none) {}
            else if(r < p_none+p_1) pkt.at1=true;
            else if(r < p_none+p_1+p_2) pkt.at2=true;
            else if(r < p_none+p_1+p_2+p_3) pkt.at3=true;
            else if(r < p_none+p_1+p_2+p_3+p_12){ pkt.at1=true; pkt.at2=true; }
            else if(r < p_none+p_1+p_2+p_3+p_12+p_13){ pkt.at1=true; pkt.at3=true; }
            else if(r < p_none+p_1+p_2+p_3+p_12+p_13+p_23){ pkt.at2=true; pkt.at3=true; }
            else { pkt.at1=true; pkt.at2=true; pkt.at3=true; }
        }

        /* ===== STAGE 2 ===== */
        while(!(pkt.d4 && pkt.d5 && pkt.d6)){

            double r1 = urand();
            double r2 = urand();
            double r3 = urand();

            bool send1 = pkt.at1 && (!pkt.d4 || !pkt.d5);
            bool send2 = pkt.at2 && (!pkt.d5 || !pkt.d6);
            bool send3 = pkt.at3 && (!pkt.d5 || !pkt.d6);

            if(send1) transmissions++;
            if(send2) transmissions++;
            if(send3) transmissions++;

            if(send1){
                if(r1 >= none14){
                    if(r1 < none14 + only4) pkt.d4=true;
                    else if(r1 < none14 + only4 + only5_1) pkt.d5=true;
                    else{ pkt.d4=true; pkt.d5=true; }
                }
            }

            if(send2){
                if(r2 >= none25){
                    if(r2 < none25 + only5_2) pkt.d5=true;
                    else if(r2 < none25 + only5_2 + only6_2) pkt.d6=true;
                    else{ pkt.d5=true; pkt.d6=true; }
                }
            }

            if(send3){
                if(r3 >= none35){
                    if(r3 < none35 + only5_3) pkt.d5=true;
                    else if(r3 < none35 + only5_3 + only6_3) pkt.d6=true;
                    else{ pkt.d5=true; pkt.d6=true; }
                }
            }
        }

        delivered++;
    }

    cout<<"Simulation ETX = "<<(double)transmissions/N<<"\n";

    return 0;
}