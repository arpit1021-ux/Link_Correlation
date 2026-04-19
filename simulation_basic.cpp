#include <iostream>
#include <ctime>
#include <iomanip>
#include <algorithm>
using namespace std;

/* ========= RANDOM GENERATOR ========= */
#define IA1 40014
#define IA2 40692
#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
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

int main() {
    cout<<fixed<<setprecision(6);

    double p01,p02,p12;
    double p13,p23;
    int N;

    cout<<"p(0->1): "; cin>>p01;
    cout<<"p(0->2): "; cin>>p02;
    cout<<"p(0->1,0->2): "; cin>>p12;
    cout<<"p(1->3): "; cin>>p13;
    cout<<"p(2->3): "; cin>>p23;
    cout<<"Packets: "; cin>>N;

    /* Region probabilities */
    double none = 1 - p01 - p02 + p12;
    double only1 = p01 - p12;
    double only2 = p02 - p12;
    double both = p12;

    /* Timeline */
    double T0 = none;
    double T1 = T0 + only1;
    double T2 = T1 + only2;

    /* Costs */
    double c13 = 1.0/p13;
    double c23 = 1.0/p23;

    long seed = -time(NULL);
    long total_tx = 0;

    for(int i=0;i<N;i++){
        bool r1=false, r2=false, r3=false;
        int tx=0;

        while(!r3){
            tx++;
            float r = randomgeneration(seed);

            if(!r1 && !r2){
                if(r<T0){}
                else if(r<T1) r1=true;
                else if(r<T2) r2=true;
                else r1=r2=true;
            } else {
                double best_p = max(r1?p13:0, r2?p23:0);
                if(randomgeneration(seed)<best_p) r3=true;
            }
        }
        total_tx+=tx;
    }

    double simETX = (double)total_tx/N;

    double anaETX =
        ( none
        + only1*(1+c13)
        + only2*(1+c23)
        + both*(1+min(c13,c23))
        )/(1-none);

    cout<<"\nAnalytical ETX = "<<anaETX;
    cout<<"\nSimulated  ETX = "<<simETX<<"\n";
}