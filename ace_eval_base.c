/* Mini poker hand evaluator.
 * Takes a hand of 5-7 cards, 
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
//#define c(a)for(X=a,i=C=0;X;X/=4}C|=(X&1)<<i++;
Card compress(Card a){
  int i=0;
  Card out=0;
  for (i;a;i++){
	 out|=(a&1)<<i;
	 a/=4;
  }
  return out/8;
}
	 

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
Card E(Card h[]){ 
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
  Card temp;


/* Quad detector: the value `v=e&o/2` will be non-zero only if a rank has both 
   the even and odd bit set, meaning its count-1 is 3.
   The while loop clears all except the top bit of the remaining to find the kicker `k`.  
	Type is stored in `t`
 */
  if(value=evens&odds/2){
	 result=7;
	 kicker=h[3]^value;
	 while(temp=kicker&kicker-1)
		kicker=temp;
  }

/* Full House detector:  
	The first line catches 2 sets (odds counter has 2 bits set).
	 It separates the bits into high set, in `value` and the pair in `k`
   The the second line catches a set plus one or two pairs. 
     It clears one bit from the pairs field if needed when setting `k`
     (since AAAKKQQ ranks the same as AAAKKQJ)
*/
  else if(value=odds&odds-1){
	 result=6;
	 value/=2;
	 kicker=(odds/2)^value;
  } 
  else if (evens&&odds) {
	 result=6;
	 value=odds/2;
	 temp = evens&evens-1;
	 kicker= (temp)?temp:evens;
  } 

/*  All the other hands fall here.  
    `h[3]` is in `k`, it will be used to detect straights.
    (it  holds a bit for each unique value and a bit for each unique suit)
*/
 else{
  /*	Look for flushes.
		remember that for suit X=1,2,4,8: h[X&7] holds a 3-bit card count, 
        starting at bit 0,1,2,3 respectively
		subtract 1 from the count, store in `C`
		If C>4, we have a 5 card flush. `t` is 5. 
		overwrite `k` with the flush suit, since a plain straight won't beat this, but a
		straight flush will.
  */

	for (i=0;i<4;i++){
	  int idx  = (1<<i)&7;
	  count = h[idx]>>i;
	  count &= 7;
	  if(count>=5){
		 kicker=h[idx];
		 result=5;
		 break;
	  }
	}
	  //   printf("#%d %08x %08x %d\n",C,k,h[X&7],X);
	  

	//   printf("#%d %08x %d %d\n",C,k,X,i);

/* Now the straight detector. 
   clear the suit bits from a, then copy down the high bit (ace) 
	   to the ones position so we can catch 5-high straights.
*/
  kicker&=-64;
  value=kicker|(kicker>>26)&16; 
  
/* The next line zeros value unless there are at least 5 cards in a row.  
   `t` will be 4 for straights, 9 for straight flushes.
    For a 6 or 7 card straight, there will be multiple consecutive bits set in value: 
	    `value&=~(value/4)` clears all but the highest. 
*/
  for (i=0;i<4;i++){
	 value&=value*4;
  }
  if(value){
	 result+=4;
	 value&=~(value/4);
	 kicker=value;
  }
  //k^value has 0 bits, i does not matter
/* finish up the flush processing: 't' is only set for flush, 
   store the high 5 cards in `k` and `value`, 
	by clearing low bit until the card count `C` is 5.
   (done after straight detection to avoid calling AK98765 in same suit a plain flush.)
  ((i will be 0 for cases below here))
 */
//  else if(i=t){for(i=(h[v&7]&63)/v;i-->5;)k&=k-1;v=k;} //k^v has 0 bits, i does not matter
  else if (i=result){
	 while(count-->5){
		kicker&=kicker-1; //k^v has 0 bits, i does not matter
	 }
	 value = kicker;
  }
/* three of a kind:
	two sets are a full house, caught above. so if there is any bit left in 'odds',
	it is a set. v=o/2 shifts the value bit into the right place
*/
  else if(value=odds/2) {
	 result=3;     //v has 1 bit, k^v has 4 bits, i is 0 so we can clear 2  
  }
/* Pairs:  a bit set in evens is a pair.  we might have 1,2, or 3 of them.
   `o` will be set if there is more than one, `i` will be set if there are 3. 
    `v` is set to the top 1 or 2 cards. 't' is 1 or 2.

 */
  else if (evens){
	 odds=evens&evens-1;
	 i=odds&odds-1;
	 value=(temp)?odds:evens;
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
  kicker&=kicker-1;
  kicker&=kicker-(i==0);
 } 
/*
  build the final result.  
  4 bits for the type 0..9, 13 bits for the value cards, 13 for the kicker.
 */
  value=compress(value);
  C=compress(kicker); 
  return result<<28|value<<13|C;
} 


