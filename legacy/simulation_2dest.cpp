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

/* ========= DEPENDENT MULTICAST ETX ========= */
double etx_two_receivers(double pi, double pj, double pij) {
    double ci = 1.0/pi, cj = 1.0/pj;
    double none = 1 - pi - pj + pij;
    double onlyi = pi - pij;
    double onlyj = pj - pij;
    double both = pij;

    return ( none
           + onlyi*(1+cj)
           + onlyj*(1+ci)
           + both
           ) / (1 - none);
}

int main() {
    cout<<fixed<<setprecision(6);

    double p01,p02,p12;
    double p13,p14,p134;
    double p23,p24,p234;
    int N;

    cout<<"p(0->1): "; cin>>p01;
    cout<<"p(0->2): "; cin>>p02;
    cout<<"p(0->1,0->2): "; cin>>p12;

    cout<<"p(1->3): "; cin>>p13;
    cout<<"p(1->4): "; cin>>p14;
    cout<<"p(1->3,1->4): "; cin>>p134;

    cout<<"p(2->3): "; cin>>p23;
    cout<<"p(2->4): "; cin>>p24;
    cout<<"p(2->3,2->4): "; cin>>p234;

    cout<<"Packets: "; cin>>N;

    /* Upstream regions */
    double none = 1 - p01 - p02 + p12;
    double only1 = p01 - p12;
    double only2 = p02 - p12;
    double both = p12;

    /* Downstream costs */
    double R1 = etx_two_receivers(p13,p14,p134);
    double R2 = etx_two_receivers(p23,p24,p234);

    /* Analytical ETX */
    double anaETX =
        ( none
        + only1*(1+R1)
        + only2*(1+R2)
        + both*(1+min(R1,R2))
        )/(1-none);

    /* Simulation */
    long seed=-time(NULL), total_tx=0;

    for(int i=0;i<N;i++){
        bool r1=false,r2=false,d3=false,d4=false;
        int tx=0;

        while(!(d3&&d4)){
            tx++;
            float r=randomgeneration(seed);

            if(!r1&&!r2){
                if(r<none){}
                else if(r<none+only1) r1=true;
                else if(r<none+only1+only2) r2=true;
                else r1=r2=true;
            } else {
                if(r1){
                    if(!d3 && randomgeneration(seed)<p13) d3=true;
                    if(!d4 && randomgeneration(seed)<p14) d4=true;
                }
                if(r2){
                    if(!d3 && randomgeneration(seed)<p23) d3=true;
                    if(!d4 && randomgeneration(seed)<p24) d4=true;
                }
            }
        }
        total_tx+=tx;
    }

    cout<<"\nAnalytical ETX = "<<anaETX;
    cout<<"\nSimulated  ETX = "<<(double)total_tx/N<<"\n";
}