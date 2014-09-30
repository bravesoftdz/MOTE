#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <math.h>
#include "cktype.h"
#include "csprng.h"

//#define TEST 
//#define MOD
//#define LIM
#define SAM

#define	MODU 26
#define	MODM1 MODU-1
#define	MODBITS 5

enum CSPRNG	rng = BB512;
const char *RNGs[11] = { "ISAAC", "MOTE8", "MOTE16", "MOTE32", "BB128", "BB256", "BB512", "MITE8", "MITE16", "MITE32", "MITE64" };

static ub4	r,m;
long int 	i,j;
static char	s[MAXK] = "Monte Carlo Mod";
static ub8	q,qi;
static ub8	totals[MODU];		
double		prob[MODU];
double		expect[MODU];
double		probtot,range;
char		values[MODU];


// count the bits in a number
static ub4 bitCount(ub4 val) { 
    register ub4 v,c=0;
	v = val;
	while (v > 0) { 
		c++; v = v >> 1;
	}
	return c;
}


// print a binary string followed by chosen escape character
// usage: putb(sizeof(n),&n,'\n');
static void putb(size_t const size, void const * const ptr, char esc)
{
    unsigned char *b = (unsigned char*) ptr;
    register unsigned char byte;
    register int i, j;
    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
    printf("%c",esc);
}


/* Obtain a value in [0..val-1] from a pseudo-random bitstream
   by sampling b-bit segments as n until n is in range. 
   < Like other alternatives to MOD, SAM exhibits a slightly 
    inferior distribution over the range of possible values
    while Sigma approaches zero more slowly than with MOD.
    tcc : SAM w/ MODBITS constant is the same speed as MOD; 
    cl  : SAM w/ MODBITS constant is 1.1 to 1.5 x faster than MOD.
    Given a high quality RNG, SAM is otherwise unbiased. > */ 
static ub4 Sam(enum CSPRNG ng) {
	register ub4 m,n,b,r;
	register long int i;
	int f=0;
	b = MODBITS;
	for(;;) {
		r = rRandom(ng);
		i = -b;
		for(;;) {
			i+=b;
			m = ((1 << b) - 1) << i;
			n = (r & m) >> i;
			f = n<MODU;
			if (f || i>=32-b) break;
		}
		if (f) break;
	}
	return n;
}


/* Obtain a value in [0..val-1] from a PRNG.
   < Like other alternatives to MOD, LIM exhibits a slightly 
    inferior distribution over the range of possible values
    while Sigma approaches zero more slowly than with MOD. */
static ub4 Lim(enum CSPRNG ng) {
    register ub4 dv = 0xFFFFFFFF/MODU;
    register ub4 n;
    do { 
        n = rRandom(ng) / dv;
    } while (n > MODM1);
    return n;
}


// Maximum from a set of probabilities (returns array index)
static ub4 MaxP(ub4 mins,ub4 maxs) {
	register ub4 i,max;
	double m = 0.0;
	for (i=mins; i<=maxs; i++)
		if (prob[i]>m) { m=prob[i]; max=i; }
	return max;
}	

	
// Minimum from a set of probabilities (returns array index)
static ub4 MinP(ub4 mins,ub4 maxs) {
	register ub4 i,min;
	double m = 1.0;
	for (i=mins; i<=maxs; i++)
		if (prob[i]<m) { m=prob[i]; min=i; }
	return min;
}	


// Mean of a set of probabilities
static double Mean(ub4 mins,ub4 maxs) {
	register i,dv;
	double t = 0.0;
	dv   = maxs-mins+1;
	for (i=mins; i<=maxs; i++) t += prob[i];
	return (double) t/dv;
}


// Standard deviation of a set of probabilities
static double Sigma(ub4 minx,ub4 maxs) {
	register ub4 i,dv;
	double	m, t=0.0;
	double d[MODU];
	dv   = maxs-minx+1;
	m = Mean(minx,maxs);
	for (i=minx; i<=maxs; i++) 
			d[i] = pow((prob[i]-m),2);
	// variance 
	for (i=minx; i<=maxs; i++) t += d[i];
	m = (double) t/dv;	
	return sqrt(m);
}



int main(int argc, char *argv[]) {
	ub4 tots=0;
	ub4 ts, r, bts, div;
	ub4 q1 = 24;
	ub4 q2 = 28;
	double totSigma=0.0;
	double totRange=0.0;
	double sigma,range,mSigma,mRange;
	// timer
	time_t a,z;
	// rounds
	qi = pow(2,q1);
	// check the command line
	if (argc>=2) q1 = atoi(argv[1]);
	if (argc>=3) q2 = atoi(argv[2]);
	if (argc>=4) rng= atoi(argv[3]) % 11;
	if (argc>=5) strcpy(s,argv[4]);
	
	#ifdef TEST
	#ifdef __TINYC__
		puts("Tiny C");
	#endif
	#ifdef __WATCOMC__
		puts("Open Watcom C");
	#endif
	#ifdef _MSC_VER
		puts("Microsoft Visual C");
	#endif
	#ifdef __GNUC__
		puts("GNU C");
	#endif
	#endif
	#if __STDC_VERSION__ >= 199901L
		puts("C99 supported.");
	#endif

	#ifdef MOD
	printf("MOD: ");
	#endif
	#ifdef LIM
	printf("LIM: ");
	#endif
	#ifdef SAM
	printf("SAM: ");
	#endif

	div = q2-q1+1;
	printf("%d %s trial sets in [2**%d..2**%d]\n",div,RNGs[rng],q1,q2);
		
	puts("Trial      Range        Sigma         Time");
	puts("------------------------------------------");
	
	for (j=q1; j<=q2; j++) {
		for (i=0; i<MODU; i++) totals[i]=0;
		probtot = 0.0;
		qi = pow(2,j);
		rSeed(rng,s,rStateSize(rng)*7);
		time(&a);
		
		for (q=0; q<qi; q++) { 
			#ifdef MOD
			r=rRandom(rng) % MODU;
			#else
			#ifdef LIM
			r=Lim(rng);
			#endif
			r=Sam(rng);
			#endif
			totals[r]++;
		}
		
		time(&z);
		ts=(size_t)(z-a);
		tots+=ts;
	
		for (i=0; i<MODU; i++) {
			// expected probabilities
			expect[i] = (double)1/MODU;
			// actual probabilities
			prob[i]=(double)totals[i]/qi;
			// probtot holds total of probabilities - it should converge to 1.0
			probtot=probtot+prob[i];
			// collect value-names & decide output format
			values[i] = i + 'A';
		}
		sigma = Sigma(0,MODM1);
		range = prob[MaxP(0,MODM1)]-prob[MinP(0,MODM1)];
		totSigma+=sigma; totRange+=range;
		printf("2**%d    %1.7f    %1.7f      %3d s\n",j,range,sigma,ts);
	}
	puts("------------------------------------------");
	mSigma = (double)totSigma/div; mRange = (double)totRange/div;
	    printf("Mean:    %1.7f    %1.7f     %4d s\n",mRange,mSigma,tots);
	#ifdef TEST
	puts(""); for (i=0; i<MODU; i++) printf("%2c) %1.7f\n",values[i],prob[i]);
	#endif
	return 0;
}
