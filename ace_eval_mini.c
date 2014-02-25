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

/* some shorteners:
 */
#define L(n)for(i=0;i<n;i++)
#define U unsigned 

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
U C,i,X; 

/* The evaluator function:*/
U E(U h[]){ 
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
  U e=0x55555540,k=h[3],a=*h+h[1]+h[2]+h[4]-(k&-16L),o=2*e&a,t=0,v;e&=a;

/* Quad detector: the value `v=e&o/2` will be non-zero only if a rank has both 
   the even and odd bit set, meaning its count-1 is 3.
   The while loop clears all except the top bit of the remaining to find the kicker `k`.  
	Type is stored in `t`
 */
if(v=e&o/2){t=7;k^=v;while(i=k&k-1)k=i;}

/* Full House detector:  
	The first line catches 2 sets (odds counter has 2 bits set).
	 It separates the bits into high set, in `v` and the pair in `k`
   The the second line catches a set plus one or two pairs. 
     It clears one bit from the pairs field if needed when setting `k`
     (since AAAKKQQ ranks the same as AAAKKQJ)
*/
else if(v=o&o-1){t=6;v/=2;k=o/2^v;} 
else if(e&&o){t=6;v=o/2;k=(i=e&e-1)?i:e;} 

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


	L(4) if((C=((a=h[(1<<i)&7])>>i)&7)>4){k=a;t=5;break;}
	
	  //   printf("#%d %08x %08x %d\n",C,k,h[X&7],X);
	  

	//   printf("#%d %08x %d %d\n",C,k,X,i);

/* Now the straight detector. 
   clear the suit bits from a, then copy down the high bit (ace) 
	   to the ones position so we can catch 5-high straights.
*/
  k&=-64;v=k|k>>26&16; 
/* The next line zeros v unless there are at least 5 cards in a row.  
   `t` will be 4 for straights, 9 for straight flushes.
    For a 6 or 7 card straight, there will be multiple consecutive bits set in v: 
	    `v&=~(v/4)` clears all but the highest. 
*/
  L(4)v&=v*4;if(v){t+=4;k=v&=~(v/4);}  //k^v has 0 bits, i does not matter
/* finish up the flush processing: 't' is only set for flush, 
   store the high 5 cards in `k` and `v`, 
	by clearing low bit until the card count `C` is 5.
   (done after straight detection to avoid calling AK98765 in same suit a plain flush.)
  ((i will be 0 for cases below here))
 */
//  else if(i=t){for(i=(h[v&7]&63)/v;i-->5;)k&=k-1;v=k;} //k^v has 0 bits, i does not matter
  else if(i=t)while((v=k)&&C-->5)k&=k-1; //k^v has 0 bits, i does not matter
/* three of a kind:
	two sets are a full house, caught above. so if there is any bit left in 'odds',
	it is a set. v=o/2 shifts the value bit into the right place
 */
  else if(v=o/2)t=3;     //v has 1 bit, k^v has 4 bits, i is 0 so we can clear 2  
/* Pairs:  a bit set in evens is a pair.  we might have 1,2, or 3 of them.
   `o` will be set if there is more than one, `i` will be set if there are 3. 
    `v` is set to the top 1 or 2 cards. 't' is 1 or 2.

 */
  else if (e){o=e&e-1;v=(i=o&o-1)?o:e;t=1+(o>0);}   

/* for all hands except 4 of a kind and full house,
   we have left the primary cards which determine the hand's type in 'v'
   and `a` holds all the cards (except a == v for flushes and straights)
	set k to the kickers by findig all in a not in v (a^v)
	then clear the extra 2. (or 1 if i is non zero b/c there was a 3rd pair).
 */
  printf("#%08x %08x %08x %d\n",v,k,k^v,i);
k^=v;k&=k-1;k&=k-(i==0);} 
/*
    compressor: turn 26 bit-pairs into 13 bits
*/
#define c(a)for(X=a,i=C=0;X;X/=4)C|=(X&1)<<i++;

/*
  build the final result.  
  4 bits for the type 0..9, 13 bits for the value cards, 13 for the kicker.
 */
c(v);v=C/8;c(k); 
return t<<28|v<<13|C/8;} 


