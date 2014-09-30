/* countx.c
 * By Bob Jenkins, public domain, implementing Bob Blorp2's public domain test.
	
 * Given a subsequence of n 32-bit random numbers, compute the number
 * of bits set in each term, reduce that to low, medium or high number
 * of bits, and concatenate a bunch of those 3-item buckets.
 * Do this for len overlapping n-value sequences.  And report the chi-square
 * measure of the results compared to the ideal distribution.
 
 * Extended with multiple-round tabulation by C.C.Kayne, September 2014.
	Usage: 	>countx <# rounds> <min terms> {<max terms>} {<# trials>} 
	Example:>countx 32 2 12 3
	(Perform 3 trials, from 2**32 to 2**34 sequences, from 2 to 12 terms per sequence.)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>

typedef  unsigned char      u1;
typedef  unsigned long      u4;
typedef  unsigned long long u8;
// countx globals
static u4 loglen, terms;

#define LOGBUCKETS 2
#define BUCKETS (1<<LOGBUCKETS)
#define GRAY_CODE 1

// MOTE defines
//#define TEST
//#define VERBOSE
#define TABLE

//#define MOTE8
//#define MOTE16
#define MOTE32

#ifdef MOTE8
#define STSZ 8
#endif
#ifdef MOTE16
#define STSZ 16
#endif
#ifdef MOTE32
#define STSZ 32
#endif
#define STM1 STSZ-1

/* 2**32/phi, where phi is the golden ratio */
#define GOLDEN   0x9e3779b9
#define FLEASEED 0xf1ea5eed
/* allow testing non-powers of 2 combos */
//#define MOD
#define NOT
#define DORSW
#define RI 0

typedef struct ranctx { u4 state[STSZ]; u4 rsl[STSZ]; u4 a; u4 b; u4 c; u4 d; u4 e;} ranctx;

#define rot(x,k) ((x<<k)|(x>>(32-k)))

u4 rcnt=0; u8 fcount=0; u4 icount=0;

// TEST: count of RI usages
static u4 ricnt[4];

// MOTE ROT switcher
typedef struct Rsw { u4 iii; u4 jjj; u4 kkk; };
static u4 ri=RI;
#ifdef MOTE8
#define RSW 3
static const struct Rsw rsw[RSW+1] = {
#ifndef NOT
28,  19, 16, // avalanche: 21.0 bits OK countx 2-12 (2**30)
17,  15, 28, // avalanche: 21.0 bits OK countx 2-12 (2**29)
15,  28, 12, // avalanche: 21.0 bits OK countx 2-12 (2**29)
25,   5, 20  // avalanche: 21.0 bits OK in combo
// 5,  24, 20, // avalanche: 21.0 bits
// 4,  27,  8, // avalanche: 21.0 bits
// 3,  26, 20  // avalanche: 21.0 bits
//26,  21, 28, // avalanche: 21.0 bits OK countx 2-12 (2**29) BIAS in Combo
#else
 5,  29, 14, // avalanche: 21.0 bits
 9,  20, 18, // avalanche: 21.0 bits
10,   3, 26, // avalanche: 21.0 bits
29,   5,  6  // avalanche: 21.0 bits
// 8,  27, 20, // avalanche: 21.0 bits
//18,   3, 24, // avalanche: 21.0 bits
//27,  16, 24, // avalanche: 21.0 bits
//28,  25, 26, // avalanche: 21.0 bits
#endif
#endif
#ifdef MOTE16
#define RSW 3
static const struct Rsw rsw[RSW+1] = {
#ifdef NOT
25, 10, 10, // avalanche: 20.50 bits
 8, 13, 20, // avalanche: 20.00 bits
16,  9, 24, // avalanche: 20.00 bits
 3, 18, 10, // avalanche: 19.50 bits
 6, 13,  6, // avalanche: 19.50 bits
10,  3, 18, // avalanche: 19.50 bits
13, 28, 22, // avalanche: 19.50 bits
23,  6, 20 // avalanche: 19.50 bits
#else
29, 12, 16, // avalanche: 20.50 bits - OK 10/10 10/10 10/10; 2C:10/10
26, 16, 20, // avalanche: 19.50 bits - OK 10/10 10/10 10/10; 2C:10/10
15, 18, 12, // avalanche: 19.00 bits - OK 7/10 9/10 10/10; 3C:10/10; not w/ 14,8,28
20, 20, 16, // avalanche: 19.00 bits - OK 4C:9/10,9/10,9/10,9/10,8/10,9/10,10/10
//22, 26, 12, // avalanche: 19.00 bits - OK 4C:7/10
//26, 22,  2, // avalanche: 19.00 bits
//28, 21, 26, // avalanche: 19.00 bits
//28, 25, 22, // avalanche: 19.00 bits
//29, 25, 26, // avalanche: 19.00 bits
//30, 21, 10, // avalanche: 19.00 bits
//30, 27,  2, // avalanche: 19.00 bits
//30, 30, 24, // avalanche: 19.00 bits
//16, 21,  4, // avalanche: 19.00 bits - OK 9/10 9/10 10/10; 3C:7/10
//14,  8, 28, // avalanche: 19.00 bits - OK 9/10 6/10 9/10; 2C:10/10, 3C:8/10; not w/ 15,18,12
#endif
#endif
#ifdef MOTE32
#define RSW 3
static const struct Rsw rsw[RSW+1] = {
#ifndef NOT
30,  4, 20, // avalanche: 19.00 bits
11,  2, 26, // avalanche: 18.75 bits
19, 19, 14, // avalanche: 18.75 bits
 6, 16, 12  // avalanche: 18.25 bits
// 3, 27, 30, // avalanche: 19.00 bits
// 6, 28, 18, // avalanche: 18.50 bits
//10, 18, 10, // avalanche: 18.75 bits
//11,  2, 26, // avalanche: 18.75 bits
//15, 23, 22, // avalanche: 18.50 bits
//19, 19, 14, // avalanche: 18.75 bits
//25,  1, 24, // avalanche: 19.00 bits
//30,  1, 16, // avalanche: 19.00 bits
//30,  4, 20, // avalanche: 19.00 bits
//30,  8, 12, // avalanche: 18.50 bits
#else
30,  4, 20, // avalanche: 19.00 bits
11,  2, 26, // avalanche: 18.75 bits
19, 19, 14, // avalanche: 18.75 bits
 6, 16, 12  // avalanche: 18.25 bits
#endif
#endif
};

// MOTE PRNG
static u4 ranval(ranctx *x) {
	register u4 i,r;
	r = x->rsl[rcnt];
	++rcnt; 
	#ifdef VERBOSE
	++fcount;
	#endif
	if (rcnt==STSZ) {
		for (i=0; i<STSZ; i++) {
			#ifndef NOT
			x->state[x->c & STM1] = x->e;
			#else
			x->state[x->c & STM1] = ~x->e;
			#endif
			#ifdef PLUS
			x->b = x->c ^ ((x->e>>rsw[ri].iii) + (x->d<<rsw[ri].jjj));
			#else
			x->b = x->c ^ ((x->e>>rsw[ri].iii) | (x->d<<rsw[ri].jjj));
			#endif
			x->c = x->d - rot(x->b,rsw[ri].kkk);
			x->d = x->state[i] + x->b;
			x->e = x->c + x->d;
			x->rsl[i] = x->c;
		}
		rcnt = 0;
		#ifdef DORSW
		#ifdef MOD
		ri = x->d % (RSW+1);
		#else
		ri = x->d & RSW;
		#endif
		#ifdef TEST
		ricnt[ri]++;
		#endif
		#endif
	}
	return r;
}

static void raninit(ranctx *x, u4 seed) {
	register u4 i,r;
	#ifdef VERBOSE
	icount++;
	#endif
	x->e = x->b = x->c = x->d = FLEASEED;
	for (i=0; i<STSZ; i++) { x->state[i]=GOLDEN; x->rsl[i]=0; }
	x->state[0]=seed;
	// warmup
	for (i=0; i<20; i++) r = ranval(x);
}


/* count how many bits are set in a 32-bit integer, returns 0..32 */
static u4 count(u4 x)
{
  u4 c = x;

  if (GRAY_CODE) c = c^(c<<1);

   c = c - ((c >> 1) & 0x55555555);
   c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
   c = (c + (c >> 4)) & 0x0F0F0F0F;
   c = c + (c >> 8);
   c = c + (c >> 16);
   return c & 0x0000003F;
}

/* somehow covert 0..32 into 0..BUCKETS-1 */
static u4 ftab[] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1,
  1, 
  1, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2
};

/* initialize the data collection array */
static void datainit2( u8 *data, u4 index, u4 depth, u4 terms)
{
  u4 i;
  index *= 3;
  if (depth == terms-1) {
    for (i=0; i<3; ++i)
      data[index+i] = 0;
  } else {
    for (i=0; i<3; ++i)
      datainit2(data, index+i, depth+1, terms);
  }
}

static void datainit( u8 *data, u4 terms)
{
  u4 i;
  for (i=0; i<(1<<(LOGBUCKETS*terms)); ++i)
    data[i] = 0;
}

/* gather statistics on len overlapping subsequences of "terms" values each */
static void gather( ranctx *r, u8 *data, u8 len, u4 terms)
{
  u8 i;
  u4 val = 0;
  u4 mask = (1<<(LOGBUCKETS*terms))-1;
  for (i=0; i<BUCKETS; ++i)
    val = ((val<<LOGBUCKETS)&mask) + ftab[count(ranval(r))];
  for (i=0; i<len; ++i) {
    val = ((val<<LOGBUCKETS)&mask) + ftab[count(ranval(r))];
    ++data[val];
  }
}

/* figure out the probability of 0..BUCKETS-1=ftab[count(u4)] */
static void probinit( double *pc, u4 maxbits)
{
  u8 i,j,k;
  for (i=0; i<=maxbits; ++i) {
    pc[i] = 0.0;
  }
  for (i=0; i<=maxbits; ++i) {
    k = 1;
    for (j=1; j<=i; ++j) {
      k = (k * (maxbits+1-j)) / j;
    }
    pc[ftab[i]] += ldexp((double)k,-32);
  }
}

#define MAXBITS 32
static void chi( u8 *data, u8 len, u4 terms)
{
  u4 i,j,k;                 /* counters */
  double pc[MAXBITS+1];     /* pc[i] is probability of a bitcount of i */
  double expect = 0.0;      /* expected fullness of current bucket */
  double expectother = 0.0; /* expected fullness of "other" bucket */
  double var = 0.0;         /* total variance */
  double temp;              /* used to calculate variance of a bucket */
  u8 buckets = 0;           /* number of buckets used */
  u8 countother = 0;
    
  probinit(pc, MAXBITS);

  /* handle the nonnegligible buckets */
  for (i=0; i < (1<<(LOGBUCKETS*terms)); ++i) {
    
    /* determine the expected frequency of this bucket */
    expect = (double)len;
    k = i;
    for (j=0; j<terms; ++j) {
      expect *= pc[k&(BUCKETS-1)];
      k >>= LOGBUCKETS;
    }

    /* calculate the variance for this bucket */
    if (expect < 5.0) {
      expectother += expect;
      countother += data[i];
    } else {
      ++buckets;
      temp = (double)data[i] - expect;
      temp = temp*temp/expect;
      if (temp > 20.0) {
	k = i;
	for (j=0; j<terms; ++j) {
	  #ifdef VERBOSE
	  printf("%2d ", k&(BUCKETS-1));
	  #endif
	  k >>= LOGBUCKETS;
	}
	#ifdef VERBOSE
	printf("%14.4f %14.4f %14.4f\n",
	       (float)temp,(float)expect,(float)data[i]);
	#endif
      }
      var += temp;
    }
  }

  /* lump all the others into one bucket */
  if (expectother > 5.0) {
    ++buckets;
    temp = (double)countother - expectother;
    temp = temp*temp/expectother;
    if (temp > 20.0) {
      #ifdef VERBOSE
	  printf("other %14.4f %14.4f %14.4f\n",
	     (float)temp,(float)expectother,(float)countother);
	  #endif	
    }
    var += temp;
  }
  --buckets;

  /* calculate the total variance and chi-square measure */
  #ifdef TABLE
  printf("2**%2d	%2d	%10.2f	%10.2f	%8.4f",loglen,terms,(float)buckets,(float)var, 
	 (float)((var-buckets)/sqrt((float)buckets)));
  #else
  printf("expected variance: %11.4f   got: %11.4f   chi-square: %6.4f\n",
         (float)buckets, (float)var, 
	 (float)((var-buckets)/sqrt((float)buckets)));
  #endif
}


int main( int argc, char **argv)
{
  u8 len;
  u8 *data;
  u4 i,round,rounds,minterms,maxterms,trial,trials=1;
  ranctx r;
  time_t a,z;
  
  if (argc >= 3) {
	sscanf(argv[1], "%d", &loglen);
	#ifdef VERBOSE
	printf("sequence length: 2^^%d\n", loglen);
	#endif

	sscanf(argv[2], "%d", &terms);
	#ifdef VERBOSE
	printf("terms in subsequences: %d\n", terms);
	#endif
	} else {
		fprintf(stderr, "usage: \"countx 24 6\" means use 2^^24 sequences of length 6\n");
		return 1;
	}
	if (argc>=4) { maxterms=atoi(argv[3]); rounds=maxterms-terms+1; } else rounds=1;
	if (argc>=5) trials=atoi(argv[4]);
	
	minterms=terms;
	
	// trials loop
	for (trial=0; trial<trials; trial++) {
		printf("\n%s: ",argv[0]);
		#ifdef MOTE8
		printf("MOTE8");
		#endif
		#ifdef MOTE16
		printf("MOTE16");
		#endif
		#ifdef MOTE32
		printf("MOTE32");
		#endif
		#ifdef NOT
		printf("N");
		#endif
		#ifdef DORSW
		printf(" RSW(%d) ",RSW+1);
		for (i=0;i<=RSW;i++) printf("{%2d,%2d,%2d}",rsw[i].iii,rsw[i].jjj,rsw[i].kkk);
		#else
		printf(" (RI=%d) {%2d,%2d,%2d}",RI,rsw[RI].iii,rsw[RI].jjj,rsw[RI].kkk);
		#endif
		puts("");

	
		#ifdef TABLE
		printf("Trial	Terms	Expected V	Observed V	Chi-square	 Time\n");
		printf("----------------------------------------------------------------------\n");
		#endif
		
		len = (((u8)1)<<loglen);
		terms=minterms;
		
		// sequences loop
		for (round=0; round<rounds; round++) {
			time(&a);

			data = (u8 *)malloc(sizeof(u8)*(1<<(LOGBUCKETS*terms)));
			if (!data) {
				fprintf(stderr, "could not malloc data\n");
				return 1;
			}

			for (i=0; i<=MAXBITS; ++i) {
				if (ftab[i] > BUCKETS) {
				fprintf(stderr, "ftab[%d]=%d needs a bigger LOGBUCKETS\n", i, ftab[i]);
				return 1;
				}
			}

			datainit(data, terms);
			raninit(&r, 0);
			gather(&r, data, len, terms);
			chi(data, len, terms);

			free(data);

			time(&z);
			#ifdef TABLE
			printf("	%4d s\n",(size_t)(z-a));
			#else
			printf("\nnumber of seconds      : %12d\n", (size_t)(z-a));
			#endif
			#ifdef VERBOSE
			printf("number of ranval calls : %12llu\n", fcount);
			printf("number of raninit calls: %12u\n", icount);
			#endif
		
			terms++;
		}
		loglen++;
	}

	#ifdef TEST
	#ifdef DORSW
	puts(""); for (i=0; i<=RSW; i++) printf("RI[%u] %u; ",i,ricnt[i]); puts("");
	#endif
	#endif
	return 0;
}

