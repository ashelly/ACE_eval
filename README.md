ACE_eval
========


**ACE Card Evaluator**

*(or maybe AShelly's Card Evaluator)*




### What's this? 
It is indeed Yet Another **Poker Evaluator**. Specifically it is a **7-card hand evaluator** implemented in **under 600 bytes** of C source code.  It is not the tiniest evaluator, and certainly not the fastest, but the goal is to make it among the fastest tiny ones, or maybe the tiniest fast one.  

### How To Use It?.
In 4 simple steps:

1. Initialize the deck with 52 calls to `Card ACE_makecard(int n);` 
	This creates the internal card representation
2. Create an empty hands with `Card hand[ACEHAND]={0};`
3. Deal 7 cards to the hand with 7 calls to `void ACE_addcard(Card* hand, Card card);`
4. Find the hand value with `V = ACE_evaluate(Card* hand);`

- What is the hand value? 
	  A 32-bit value with the following layout:  
	  `RRRR..AKQJT98765432akqjt98765432`
          4-bit Rank, 2 spares, 13 Value Card bits, 13 Kicker bits.

It is arranged so that 

*  If V(a) > V(b) then hand a beats hand b.  Equal hands have equal values.
*  You can quickly extract both the rank and the cards that make up the hand.

Ranks:

    0:"High Card"
    1:"One Pair
    2""Two Pair",
    3:"Three of a Kind",
    4:"Straight",
    5:"Flush",
    6:"Full House",
    7:"Four of a Kind",
    8:"unused",
    9:"Straight Flush"  
    (There is no rank for "Royal Flush", since that's just a straight flush with the ace bit set.)

[`ace_decode.c`](ace_decode.c) contains example source to turn it back into a human-readable result

### What makes it different?
  It's very small, and fairly fast.

  It started out as a fairly simple evaluator in Ruby, but morphed into C at some point.  I came up with a reasonably efficient card-encoding scheme, and was happy with the results.  But then, like so many before me, I came upon the ["The Great Poker Hand Evaluator Roundup"](http://www.codingthewheel.com/archives/poker-hand-evaluator-roundup/) on Coding the Wheel, and saw its description of the [Two-plus-Two evaluator](http://archives1.twoplustwo.com/showflat.php?Cat=0&Number=8513906&page=0&fpart=1&vc=1).  So my quest for a super-fast evaluator was over before it really got started.   But I was bothered by the 123MB lookup table need by two-plus-two, so I changed my focus to writing a fast but tiny evaluator instead.

Eventually I turned it into a [StackOverflow code-golf entry](http://stackoverflow.com/a/3392025/10396). I recently dug it up and started making some tweaks. In it's tiniest form the evaluator function by itself is now **577** bytes of source code ([`ace_eval_golf.c`](ace_eval_golf.c)).

There are several supporting files that make it into an actual program:

A) The smallest useful example is [`ace_microeval.c`](ace_microeval.c), which adds code to support human-readable string input, and brings the total size to **737** bytes. 

Compiling on linux with `gcc -s -Os ace_microeval.c` creates a 6Kb executable, which you can run with:

    >  echo 3C 4C 5C 6H 8D 3D 8H | ./a.out
     20084010
    >  echo 3C 4C 5C 6H 8D 3D 8H | ./a.out | ./ace_decode
     2 Pair, Eights and Threes with a Six


B) The code for the original StackOverflow challenge is down to **894** bytes.  This takes a list of 9 cards representing a 2-player game, and returns win/lose/draw statistics. ([`so_handcomp.c`](so_handcomp.c))

C) You can verify the results with [`accuracy_test.c`](accuracy_test.c) which runs through all possible 7 card hands.

D) Test the speed with [`speed_test.c`](speed_test.c). 
   `ace_eval_golf.c` clocks in at 23.5 Million hands /second.
   See [OPTIMIZATION.md](OPTIMIZATION.md) for versions that triple the speed. 
   Fastest so far: [`ace_eval_decompress.c`](ace_eval_best.c) at **72Mhps**.

E) [`ace_golf_5.c`](ace_golf_5.c) is a version which only handles 5 card hands, reducing the size down to **424** characters.   (Plus 160 for the input handling)

### So how does it work?
Cards are stored in a 32 bit word which has the following (implied) structure:

````
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
````
or  `0a0k0q0j0t09080706050403020..SHDC`

A hand is 5 32 bit words, one for each suit, and one which has the hands or'd together, leaving a bit set each card value present.

Hands based on multiple cards are all detected using a single method:  Add the four suit words together.  This adds all the card counts in parallel.  However there's one hitch: 4 of a kind would result in a '4' which is binary 100, which overflows the 2 bits allowed.  The solution is simple: subtract the 5th word, which only has a 1 where there is a card. The result now has 'cardcount-1' for each value.  This means that the word only has bits set if there is a pair or better.

The hand is further classified by splitting into sets of high(odd) and low(even) bits and doing a few more tests. A 4-of-a-kind has both bits set for the same face value (4-1=3=0b11).  A 3-of-a-kind has only one odd bit set in the word: (3-1=2=0b10) A full house has one even and one odd bit set, or 2 odd bits (since 2 triples only counts as a full house). 1-pair has only 1 even bit, and 2-pair has 2 or 3.

Flushes are detected by examining each of the individual suit words: since cards are added into this word, the low 6 bits contains a count, offset by the position of the indicator bit. If the count is greater than or equal to  5, we have a flush, and we will use this suit instead of the whole hand for straight detection.  

Straights are indicated by having 5 set bits in a row. So shift the hand one place, and AND with itself four times, and there will only be bits left on if it is a straight.  One minor catch is that the Ace bit needs to be copied down to the '1's place for detecting 5-high straights.

That's a fairly small number of operations to get the rank of the hand.  Once the rank is determined, the final value is found by selecting the five cards that make up the hand, and separating them into rank cards and kickers, and compressiing both parts down to 13 bits.  In general the even or odd words already have the rank cards. The remaining cards are easily determined by XOR'ing those the hand. Taking the correct number of bits from there gives the kicker. 






