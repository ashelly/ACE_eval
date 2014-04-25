/* Mini poker hand evaluator.
 * Takes a hand of 5 cards, 
 * returns a 32 bit int representing its rank.
 * 
 * Cards are stored in a 32 bit word which has the following (implied) structure:

    struct card{
       unsigned num_A:2;
       unsigned num_K:2;
       unsigned num_Q:2;
       //....
       unsigned num_2:2;
       unsigned spare:2;
       unsigned spade:1;
       unsigned heart:1;
       unsigned diamond:1;
       unsigned club:1;
       };

*/
#include <stdint.h>
#include "ace_eval.h"
/*
    compressor: turn 26 bit-pairs into 13 bits
*/
Card C,i,X; 

Card compress(Card a){
  int i=0;
  Card out=0;
  for (i;a;i++){
	 out|=(a&1)<<i;
	 a/=4;
  }
  return out/8;
}
//compress: shortform
#define c(a)for(X=a,i=C=0;X;X/=4}C|=(X&1)<<i++;
	 

/* Add a card to the hand with the `A` macro: 
   The hand is stored as an array of 5 ints. The low 3 bits are used to select a suit,
   (h[0]=spade,h[1]=club,h[2]=diamond,h[4]=heart).
	The cards are added to the suit - since only one suit bit is set, 3 of the low 8 bits
   will contain a count of the cards in that suit (that's why the spare is where it is).
     h[3] has a single bit set for each card present, used for straight detection
*/
#define A(h,c)h[c&7]+=c,h[3]|=c 

/* a few globals
 */

/* The evaluator function:*/
Card Eval(Card h[]){ 
  /*variables:
	 a: the sum of all suits. counts the ranks in paralell. 
       But there are only 2 bits used to store each rank, so 4 of a kind will overflow. 
       We fix that by subtracting h[3] which has a 1 for each rank actually in the hand. 
       So now every 2-bit field holds the count-1 for that rank. 
    e: evens - it has a bit set in any rank which has a 1(pair) or 3(quad).
    o: odds - it has a bit for every 2(set) or 3(quad).
    t: type: will hold the type of hand: 
       9= stfl. straight flush.
	    7= quad. 4 of a kind
		 6= boat. full house
       5= flsh. flush
       4= run.  straight.
       3= set.  3 of a kind
       2= 2pr   2 pair
       1= pair.
       0= hi-c. high card.
     v: value - the cards that determine the hand value. (eg: pair aces vs pair kings)
     k: kicker - will hold the tiebreak card(s) (eg: KKA21 vs KKQ21)
  */            /* *h */
  Card count=h[0]+h[1]+h[2]+h[4]-(h[3]&-16L);
  Card evens=0x55555540&count;
  Card odds =0xAAAAAA80&count;
  Card result = 0;
  Card value;
  Card kicker =h[3];


/* Quad detector: the value `v=e&o/2` will be non-zero only if a rank has both 
   the even and odd bit set, meaning its count-1 is 3.
   The while loop clears all except the top bit of the remaining to find the kicker `k`.  
	Type is stored in `t`
 */
  if(value=evens&odds/2){
	 result=7;
	 kicker=h[3]^value;
  }

/* Full House detector:  
	The first line catches 2 sets (odds counter has 2 bits set).
	 It separates the bits into high set, in `value` and the pair in `k`
   The the second line catches a set plus one or two pairs. 
     It clears one bit from the pairs field if needed when setting `k`
     (since AAAKKQQ ranks the same as AAAKKQJ)
*/
  else if (evens&&odds) {
	 result=6;
	 value=odds/2;
	 kicker= evens;
  } 

/*  All the other hands fall here.  
    `h[3]` is in `k`, it will be used to detect straights.
    (it  holds a bit for each unique value and a bit for each unique suit)
*/
 else{	

  /*	Look for flushes. only 1 suit bit can be set*/
	count = kicker&15; 
  if (!count&count-1) result=+5;

/* Now the straight detector. 
   clear the suit bits from a, then copy down the high bit (ace) 
	   to the ones position so we can catch 5-high straights.
*/
  kicker&=-64;
  value=kicker|(kicker>>26)&16; 
  
/* The next line zeros value unless there are at least 5 cards in a row.  
*/
  for (i=0;i<4;i++){
	 value&=value*4;
  }
  if (value) result+=4;

  //  resul`t` will be 4 for straights, 5 for flushes, 9 for straight flushes.
  if (result)(value=kicker;}


/* three of a kind:
	two sets are a full house, caught above. so if there is any bit left in 'odds',
	it is a set. v=o/2 shifts the value bit into the right place
*/
  else if(value=odds/2) {
	 result=3;     //v has 1 bit, k^v has 4 bits, i is 0 so we can clear 2  
  }
/* Pairs:  a bit set in evens is a pair.  we might have 1 or ,2
   `o` will be set if there is more than one,
    `v` is set to the top 1 or 2 cards. 't' is 1 or 2.

 */
  else if (evens){
	 odds=evens&evens-1;
	 value=evens;
	 result=1+(odds>0);
  }   

/* for all hands except 4 of a kind and full house,
   we have left the primary cards which determine the hand's type in 'value'
   and `a` holds all the cards (except a == v for flushes and straights)
	set k to the kickers by findig all in a not in v (a^v)
	then clear the extra 2. (or 1 if i is non zero b/c there was a 3rd pair).
 */
//  printf("#%08x %08x %08x %d\n",value,k,k^value,i);
  kicker^=value;
 } 
/*
  build the final result.  
  4 bits for the type 0..9, 13 bits for the value cards, 13 for the kicker.
 */
  value=compress(value);
  C=compress(kicker); 
  return result<<28|value<<13|C;
} 

