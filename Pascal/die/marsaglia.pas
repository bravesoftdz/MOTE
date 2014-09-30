{ George Marsaglia's Random Number Generators (see comments at bottom)}
{ --------------------------------------------------------------------}
{$mode delphi}
unit marsaglia;

interface

type UL = cardinal;

function mCONG: UL; 
function mFIB:  UL; 
function mKISS: UL; 
function mMWC:  UL; 
function mSHR3: UL; 
function mLFIB4:UL;
function mSWB: UL;
function mUNI: extended; 
function mVNI: extended; 

// Initialize all the above with one seed
procedure mSeedAll(seed: UL);

// Example procedure to set the table, using KISS: */
procedure mSetTable(i1,i2,i3,i4,i5,i6: UL);

// Reset all to initial (zero) state
procedure mReset;


implementation

//  Constants for reset
CONST FIRSTz =362436069;
CONST FIRSTw =521288629; 
CONST FIRSTjsr =123456789;
CONST FIRSTjcong =380116160;
CONST FIRSTa =224466889;
CONST FIRSTb =7584631;

//  Global static variables: */
var z: UL = FIRSTz;
var w: UL = FIRSTw; 
var jsr: UL = FIRSTjsr;
var jcong: UL = FIRSTjcong;
var a: UL = FIRSTa;
var b: UL = FIRSTb;
var t: array[0..256] of UL;
// Use random seeds to reset z,w,jsr,jcong,a,b, and the table t[256]*/

var x: UL =0;
var y: UL =0;
var bro: UL=0;
var c: byte=0;


// Inline #defines converted to functions
function znew: UL; begin z:=36969*(z and 65535)+(z shr 16); result:=z; end;
function wnew: UL; begin w:=18000*(w and 65535)+(w shr 16); result:=w; end;
// Marsaglia's RNGs
function mMWC:  UL; begin result:=(znew shl 16)+ wnew; end;
function mSHR3: UL; begin jsr:=jsr xor (jsr shl 17);
	jsr:=jsr xor (jsr shr 13); jsr:=jsr xor (jsr shl 5); result:=jsr; end;
function mCONG: UL; begin jcong:=69069*jcong+1234567; result:=jcong; end;
function mKISS: UL; begin result:=(mMWC xor mCONG)+ mSHR3; end;
function mFIB: UL; begin b:=a+b; a:=b-a; result:=a; end;
function mLFIB4:UL; begin inc(c); 
	t[c]:=t[c]+t[byte(c+58)]+t[byte(c+119)]+t[byte(c+178)]; result:=t[c]; end;
function mSWB: UL; begin inc(c);
	bro:=ABS(UL(x<y)); x:=t[byte(c+34)]; y:=t[byte(c+19)]+bro; t[c]:=x-y; result:=t[c]; end;
function mUNI: extended; begin result:=mKISS*2.328306e-10; end;
function mVNI: extended; begin result:= mKISS*4.656613e-10; end;



// Example procedure to set the table, using KISS: */
procedure mSetTable(i1,i2,i3,i4,i5,i6: UL);
var i: integer; 
begin
	z:=i1 xor FIRSTz; w:=i2 xor FIRSTw; 
	jsr:=i3 xor FIRSTjsr; jcong:=i4 xor FIRSTjcong; 
	a:=i5 xor FIRSTa; b:=i6 xor FIRSTb;
 for i:=0 to 255 do t[i]:= mKISS;
end;


// Initialize all the above with seed
procedure mSeedAll(seed: UL);
begin
	mSetTable(seed,seed,seed,seed,seed,seed);
end;


// Reset all to initial (zero) state
procedure mReset;
var n: word;
begin
	// Reset table
	for n:=0 to 256 do
		t[n] := 0;
	// Reset main vars
	//  Global static variables: */
	z:=FIRSTz;
	w:=FIRSTw; 
	jsr:=FIRSTjsr;
	jcong:=FIRSTjcong;
	a:=FIRSTa;
	b:=FIRSTb;
	// Other vars
	x:=0;
	y:=0;
	c:=0;
	bro:=0;
end; {mReset}


initialization

mReset;

end.

{-----------------------------------------------------
   Write your own calling program and try one or more of
   the above, singly or in combination, when you run a
   simulation. You may want to change the simple 1-letter
   names, to avoid conflict with your own choices.        */

/* All that follows is comment, mostly from the initial
   post. You may want to remove it */

/* Any one of KISS, MWC, FIB, LFIB4, SWB, SHR3, or CONG
   can be used in an expression to provide a random 32-bit
   integer.

   The KISS generator, (Keep It Simple Stupid), is
   designed to combine the two multiply-with-carry
   generators in MWC with the 3-shift register SHR3 and
   the congruential generator CONG, using addition and
   exclusive-or. Period about 2^123.
   It is one of my favorite generators.

   The  MWC generator concatenates two 16-bit multiply-
   with-carry generators, x(n)=36969x(n-1)+carry,
   y(n)=18000y(n-1)+carry  mod 2^16, has period about
   2^60 and seems to pass all tests of randomness. A
   favorite stand-alone generator---faster than KISS,
   which contains it.

   FIB is the classical Fibonacci sequence
   x(n)=x(n-1)+x(n-2),but taken modulo 2^32.
   Its period is 3*2^31 if one of its two seeds is odd
   and not 1 mod 8. It has little worth as a RNG by
   itself, but provides a simple and fast component for
   use in combination generators.

   SHR3 is a 3-shift-register generator with period
   2^32-1. It uses y(n)=y(n-1)(I+L^17)(I+R^13)(I+L^5),
   with the y's viewed as binary vectors, L the 32x32
   binary matrix that shifts a vector left 1, and R its
   transpose.  SHR3 seems to pass all except those
   related to the binary rank test, since 32 successive
   values, as binary vectors, must be linearly
   independent, while 32 successive truly random 32-bit
   integers, viewed as binary vectors, will be linearly
   independent only about 29% of the time.

   CONG is a congruential generator with the widely used 69069
   multiplier: x(n)=69069x(n-1)+1234567.  It has period
   2^32. The leading half of its 32 bits seem to pass
   tests, but bits in the last half are too regular.

   LFIB4 is an extension of what I have previously
   defined as a lagged Fibonacci generator:
   x(n)=x(n-r) op x(n-s), with the x's in a finite
   set over which there is a binary operation op, such
   as +,- on integers mod 2^32, * on odd such integers,
   exclusive-or(xor) on binary vectors. Except for
   those using multiplication, lagged Fibonacci
   generators fail various tests of randomness, unless
   the lags are very long. (See SWB below).
   To see if more than two lags would serve to overcome
   the problems of 2-lag generators using +,- or xor, I
   have developed the 4-lag generator LFIB4 using
   addition: x(n)=x(n-256)+x(n-179)+x(n-119)+x(n-55)
   mod 2^32. Its period is 2^31*(2^256-1), about 2^287,
   and it seems to pass all tests---in particular,
   those of the kind for which 2-lag generators using
   +,-,xor seem to fail.  For even more confidence in
   its suitability,  LFIB4 can be combined with KISS,
   with a resulting period of about 2^410: just use
   (KISS+LFIB4) in any C expression.

   SWB is a subtract-with-borrow generator that I
   developed to give a simple method for producing
   extremely long periods:
      x(n)=x(n-222)-x(n-237)- borrow mod 2^32.
   The 'borrow' is 0, or set to 1 if computing x(n-1)
   caused overflow in 32-bit integer arithmetic. This
   generator has a very long period, 2^7098(2^480-1),
   about 2^7578.   It seems to pass all tests of
   randomness, except for the Birthday Spacings test,
   which it fails badly, as do all lagged Fibonacci
   generators using +,- or xor. I would suggest
   combining SWB with KISS, MWC, SHR3, or CONG.
   KISS+SWB has period >2^7700 and is highly
   recommended.
   Subtract-with-borrow has the same local behaviour
   as lagged Fibonacci using +,-,xor---the borrow
   merely provides a much longer period.
   SWB fails the birthday spacings test, as do all
   lagged Fibonacci and other generators that merely
   combine two previous values by means of =,- or xor.
   Those failures are for a particular case: m=512
   birthdays in a year of n=2^24 days. There are
   choices of m and n for which lags >1000 will also
   fail the test.  A reasonable precaution is to always
   combine a 2-lag Fibonacci or SWB generator with
   another kind of generator, unless the generator uses
   *, for which a very satisfactory sequence of odd
   32-bit integers results.

   The classical Fibonacci sequence mod 2^32 from FIB
   fails several tests.  It is not suitable for use by
   itself, but is quite suitable for combining with
   other generators.

   The last half of the bits of CONG are too regular,
   and it fails tests for which those bits play a
   significant role. CONG+FIB will also have too much
   regularity in trailing bits, as each does. But keep
   in mind that it is a rare application for which
   the trailing bits play a significant role.  CONG
   is one of the most widely used generators of the
   last 30 years, as it was the system generator for
   VAX and was incorporated in several popular
   software packages, all seemingly without complaint.

   Finally, because many simulations call for uniform
   random variables in 0<x<1 or -1<x<1, I use #define
   statements that permit inclusion of such variates
   directly in expressions:  using UNI will provide a
   uniform random real (float) in (0,1), while VNI will
   provide one in (-1,1).

   All of these: MWC, SHR3, CONG, KISS, LFIB4, SWB, FIB
   UNI and VNI, permit direct insertion of the desired
   random quantity into an expression, avoiding the
   time and space costs of a function call. I call
   these in-line-define functions.  To use them, static
   variables z,w,jsr,jcong,a and b should be assigned
   seed values other than their initial values.  If
   LFIB4 or SWB are used, the static table t[256] must
   be initialized.

   A note on timing:  It is difficult to provide exact
   time costs for inclusion of one of these in-line-
   define functions in an expression.  Times may differ
   widely for different compilers, as the C operations
   may be deeply nested and tricky. I suggest these
   rough comparisons, based on averaging ten runs of a
   routine that is essentially a long loop:
   for(i=1;i<10000000;i++) L=KISS; then with KISS
   replaced with SHR3, CONG,... or KISS+SWB, etc. The
   times on my home PC, a Pentium 300MHz, in nanoseconds:
   FIB 49;LFIB4 77;SWB 80;CONG 80;SHR3 84;MWC 93;KISS 157;
   VNI 417;UNI 450;
 
   POSTED BEFORE CODE
   ------------------
   My offer of RNG's for C was an invitation to dance;
I did not expect the Tarantella.  I hope this post will
stop the music, or at least slow it to a stately dance
for language chauvinists and software police---under
a different heading.

In response to a number of requests for good RNG's in
C, and mindful of the desirability of having a variety
of methods readily available, I offered several. They
were implemented as in-line functions using the #define
feature of C.

Numerous responses have led to improvements; the result
is the listing above, with comments describing the
generators.

I thank all the experts who contributed suggestions, either
directly to me or as part of the numerous threads.

It seems necessary to use a (circular) table in order
to get extremely long periods for some RNG's. Each new
number is some combination of the previous r numbers, kept
in the circular table.  The circular table has to keep
at least the last r, but possible more than r, numbers.

For speed, an  8-bit index seems best for accessing
members of the table---at least for Fortran, where an
8-bit integer is readily  available via integer*1, and
arithmetic on the index is automatically mod 256
(least-absolute-residue).

Having little experience with C, I got out my little
(but BIG) Kernighan and Ritchie book to see if there
were an 8-bit integer type. I found none, but I did
find char and unsigned char: one byte. Furthemore, K&R
said arithmetic on characters was ok. That, and a study
of the #define examples, led me to propose #define's
for in-line generators LFIB4 and SWB, with monster
periods. But it turned out that char arithmetic jumps
"out of character", other than for simple cases such as
c++ or c+=1.   So, for safety, the index arithmetic
below is kept in character by the UC definition.

Another improvement on the original version  takes
advantage of the comma operator, which, to my chagrin,
I had not seen in K&R.  It is there, but only with an
example of (expression,expression).  From the advice of
contributors, I found that the comma operator allows
(expression,...,expression,expression) with the
last expression determining the value.   That makes it
much easier to create in-line functions via #define
(see SHR3, LFIB4, SWB and FIB below).

The improved #define's are listed below, with a
function to initialize the table and a main program
that calls each of the in-line functions one million
times and then  compares the result to what I got with
a DOS version of gcc.   That main program can serve
as a test to see if your system produces the same
results as mine.
   _________________________________________
  |If you run the program above, your output|
  | should be  seven lines, each a 0 (zero).|
   -----------------------------------------

Some readers of the threads are not much interested
in the philosophical aspects of computer languages,
but want to know: what is the use of this stuff?
Here are simple examples of the use of the in-line
functions:  Include the #define's in your program, with
the accompanying static variable declarations, and a
procedure, such as the example, for initializing
the static variable (seeds) and the table.

Then any one of those in-line functions, inserted
in a C expression, will provide a random 32-bit
integer, or a random float if UNI or VNI is used.
For example, KISS&255; would provide a random byte,
while 5.+2.*UNI; would provide a random real (float)
from 5 to 7. Or  1+MWC%10; would provide the
proverbial "take a number from 1 to 10",
(but with not quite, but virtually, equal
 probabilities).
More generally, something such as 1+KISS%n; would
provide a practical uniform random choice from 1 to n,
if n is not too big.

A key point is: a wide variety of very fast, high-
quality, easy-to-use RNG's are available by means of
the nine in-line functions above, used individually or
in combination.

The comments after the main test program describe the
generators. These descriptions are much as in the first
post, for those who missed them. Some of the
generators (KISS, MWC, LFIB4) seem to pass all tests of
randomness, particularly the DIEHARD battery of tests,
and combining virtually any two or more of them should
provide fast, reliable, long period generators. (CONG
or FIB alone and CONG+FIB are suspect, but quite useful
in combinations.)

Serious users of random numbers may want to
run their simulations with several different
generators, to see if they get consistent results.
That should be easy to do.
Bonne chance,
George Marsaglia 
 }

