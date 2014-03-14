# plan to write about optimization testing here.  

### Optimization 

The golfed evaluator `ace_eval_golf.c` takes up only 575 bytes of source code, by using many ugly tricks.  But how does it do on speed?  `speed_test.c` creates 100 million random hands, then times how long to evaluate them. 

On my current machine *(Intel® Core™ i5-3320M CPU @ 2.60GHz × 4 running Ubuntu 12.04 with gcc
v4.6.3-Target:x86_64-linux-gnu)* it runs about 23Mhps (Million hands /second).  Not bad. That means it can run through all 133 million possible hands in under 6 seconds.   But we should be able to do better. 

First, let's ungolf the names so they are human-readable, unpack the macros introduced soley for compression, and introduce a few temporary variables in place of ones reused in the interest of space.  That's `ace_eval_base.c`.  All further optimizations will work from here.   Is it any faster?  23.9Mhps.  A 3% speedup, just by giving the compiler a little more wiggle room.

Next, let's go to an age-old trick: unrolling the loops.  `ace_eval_unroll.c` unrolls the loops in the flush detector and the straight detector. It also returns directly after determining the hand type, instead of saving ther result,value cards,and kickers to the end.   This gives only a minor bump, up to 25.1Mhps.  Still it's a good idea.

The flush detection was one of the hardest part to golf efficiently, and it still seems inefficient.   Let's take a closer look at how it works.

    /*	Look for flushes.
  		remember that for suit idx=1,2,4,8: h[idx&7] holds a 3-bit card count, 
          starting at bit 0,1,2,3 respectively
  		subtract 1 from the count, store in `count`
  		If count>4, we have a 5 card flush. `result` is 5. 
  		overwrite `kicker` with the flush suit, since a plain straight won't beat this, but a
  		straight flush will.
    */

	if ((count=(h[0]>>3)&7)>4) { kicker=h[0]; result=5;} 
	else if ((count=h[1]&7)>4) { kicker=h[1]; result=5;} 
	else if ((count=(h[2]>>1)&7)>4) { kicker=h[2]; result=5;} 
	else if ((count=(h[4]>>2)&7)>4) { kicker=h[4]; result=5;} 

  
Unrolling removed the shift and mask required to index each hand, but we are still doing a shift, mask and comparison.  Maybe this is a place to break the no-tables rule to speed it up.  Is it possible to simply index all 6 bits and get the answer?   Look at h[3], which has all the cards or'd together. This would work great for a 5 card hand - a flush would have only 1 bit set.  But with a 7 card hand, it's not so useful.  If only 1 bit is set, we still have a flush, and if all 4 bits are set, we can't possibly have one, but everything else is indeterminate. We can't tell if 2 set bits represent a flush plus 2 of another suit, or 4 of one and 3 of another.   

Maybe we can add the suit bits together the same way we do with the value bits, and look them up at once.  But it's easy to find a counterexample there too:  Consider 5 Clubs, 1 Diamond, 1 Heart.  That's `b101 + b1<<1 + b1<<2 = b1011.` But 3 Clubs and 4 Diamonds is `b11 + b100<<1 = b1011` One hand is a flush, the other not, so this won't work.

So we'll definitely need to look at each of the four suit words separately. Can we at least get rid of the shifting and one 6-bit lookup table for all the words?  Nope. 6 Clubs looks the same as 3 Diamonds:  0b110.   So the shifting stays.  But aren't comparisons expensive.  Maybe we can use a lookup to avoid all those `else if`s, and just directly calculate which suit has a flush:

    static const int nc[8]={0,0,0,0,0,1,1,1};
	i=0;
	i+=nc[(h[0]>>3)&7]*8;
	i+=nc[(h[4]>>2)&7]*4;
	i+=nc[(h[2]>>1)&7]*2;
	i+=nc[(h[1]   )&7]*1;
	if (i) { kicker=h[i&7]; count=(kicker/i)&7; result=5;}  

And the time:  24.4Mhps.  Oops.  We replaced SHIFT,AND,COMPARE,ASSIGN,BRANCH with SHIFT,AND,LOOKUP,MULTIPLY,ADD. and it wasn't any faster.  Where to go from here?  Maybe where we should have started:  profiling. 

Recent versions of Linux have a great, easy-to-use profiling tool built right in.  `perf record ./time_unroll; perf report` gives an easy to use view of the hotspots.  Right away the 'compress' function jumps out.   The compiler has inlined it, but all the hotspots are in instances of that procedure. This is the function used to compress the 26 bits used to represent the card down to 13.  Let's see what we can do to improve it.  Here it is in `ace_eval_unroll.c`:

	 Card compress(Card a){
	   int i=0;
	   Card out=0;
      for (i;a;i++){
	     out|=(a&1)<<i;
	     a/=4;
      }
      return out/8;
    }

Oops. That doesn't look unrolled. How about this instead:

    Card compress(Card a){
      Card out=0;
		a>>=6; //dump low bits
		out|=a&0x1;          
		out|=(a&0x4)>>1;    
		out|=(a&0x10)>>2;   
		out|=(a&0x40)>>3;  
		out|=(a&0x100)>>4;
		out|=(a&0x400)>>5;
		out|=(a&0x1000)>>6;
		out|=(a&0x4000)>>7;
		out|=(a&0x10000)>>8;
		out|=(a&0x40000)>>9;
		out|=(a&0x100000)>>10;
		out|=(a&0x400000)>>11;
		out|=(a&0x1000000)>>12;
		out|=(a&0x4000000)>>13;
		return out;
	 }

And the new speed is... 52.0Mhps!  A 2x speedup.   The program now spends more time in `random()` shuffling decks then it does in evaluating the hands it deals.   But the compiler no longer inlines `compress`, and it is still taking up about 40% of the evaluation time.  How do we make it smaller and faster?   Let's ask [StackOverflow](	http://stackoverflow.com/q/3137266/10396).  [Matthew Slattery](https://stackoverflow.com/users/242889/matthew-slattery) came up with this version:

	 Card compress(Card a){ 
		a=(a|(a>>1))&0x33333333;
		a=(a|(a>>2))&0x0f0f0f0f;
		a=(a|(a>>4))&0x00ff00ff;
		a=(a|(a>>8))&0x0000ffff;
		return a>>3;
	 }

It works by doubling each bit, clearing every other pair of bits, then doubling the pairs and so on.   And the speedup:  72Mhps. More than 3x faster than the original.   The `compress` function is inlined again, and the `perf` profile is mostly flat.  The hotspots are concentrated around the first few lines, where it is adding the suits, and separating the even and odd bits.  This is not suprising, since every call goes through these lines, before being funneled into one or more specific cases based on the value.

Before going on to more optimization, let's find out how this code stacks up to others.

 


golf            23.0


base				 23.9
unroll			 24.1
flushtable 		 26.8
decompress      50.9
decompress2     71.8
