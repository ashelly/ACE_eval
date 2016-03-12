#include <stdio.h>
#include <stdint.h>
#include "eval.h"
#define NCARDS 7

void init_deck(uint64_t *deck) {
	for (int i = 0; i < 52; i++) 
        deck[i] = makecard(i);
}

uint64_t eval_hand(uint64_t *hand, int n) {
  int i;
  uint64_t h[5] = {0};
  for (i = 0; i < n; i++) addcard(h,hand[i]);
  return evaluate(h);
}

void verify(int c1, int c2, int c3, int c4, int c5, int c6, int c7, uint32_t value) {
    uint64_t hand[5] = {0};
    uint32_t eval;
    addcard(hand, makecard(c1));
    addcard(hand, makecard(c2));
    addcard(hand, makecard(c3));
    addcard(hand, makecard(c4));
    addcard(hand, makecard(c5));
    addcard(hand, makecard(c6));
    addcard(hand, makecard(c7));
    eval = evaluate(hand);
    if (eval == value) { printf ("ok.."); } 
    else printf("Error: %x != %x\n", value, eval);
}

static char *value_str[] = {
    "High Card",
    "One Pair",
    "Two Pair",
    "Three of a Kind",
    "Straight",
    "Flush",
    "Full House",
    "Four of a Kind",
    "",
    "Straight Flush"
};

#define hand_rank(r)       ((r)>>28)

enum cn {
    c2H, c3H, c4H, c5H, c6H, c7H, c8H, c9H, cTH, cJH, cQH, cKH, cAH,
    c2C, c3C, c4C, c5C, c6C, c7C, c8C, c9C, cTC, cJC, cQC, cKC, cAC,
    c2D, c3D, c4D, c5D, c6D, c7D, c8D, c9D, cTD, cJD, cQD, cKD, cAD,
    c2S, c3S, c4S, c5S, c6S, c7S, c8S, c9S, cTS, cJS, cQS, cKS, cAS
};

int main(void) {
    uint64_t deck[52], hand[7]={0}, freq[10]={0};
    int a, b, c, d, e, j;
    int y=0,z=-1;
    uint64_t i;
    //Test card representation
    printf("2H: %s\n", makecard(c2H) == 0x0000000000000101 ? "OK" : "ERROR");
    printf("AH: %s\n", makecard(cAH) == 0x0100000000000001 ? "OK" : "ERROR");
    printf("5C: %s\n", makecard(c5C) == 0x0000000000100002 ? "OK" : "ERROR");
    printf("QS: %s\n", makecard(cQS) == 0x0001000000000008 ? "OK" : "ERROR");

    //Verify hands
    //no hand
    verify(c2H,c3C,c4D,c5S,cTD,cJD,cKD, 0<<28|0x0000<<13|0x0B0C); 
    //1pr
    verify(cKH,c3H,c4H,c5H,cAD,cJD,cKD, 1<<28|0x0800<<13|0x1208); 
    //2pr 
    verify(cKD,c3H,c3C,c5H,cAD,cJD,cKS, 2<<28|0x0802<<13|0x1000); 
    verify(cKD,c3H,c3C,c5H,c5S,c2D,cKS, 2<<28|0x0808<<13|0x0002); 
    verify(cKD,c3H,c3C,c5H,c5S,c2D,cKS, 2<<28|0x0808<<13|0x0002); 
    //trip
    verify(c3D,c4D,c6H,c3H,c9S,c3C,cTD, 3<<28|0x0002<<13|0x0180);
    
    //straight
    verify(c3D,c4D,c9H,c5H,c2D,c3C,cAD, 4<<28|0x0008<<13|0x0000);
    verify(c3D,c4D,c6H,c5H,c2D,c3C,cAD, 4<<28|0x0010<<13|0x0000);
    verify(c3D,c4D,c6H,c5H,c2D,c7C,cAD, 4<<28|0x0020<<13|0x0000);
    verify(cTD,cJD,cKH,c5H,cQD,c3C,cAD, 4<<28|0x1000<<13|0x0000);
    
    //flush
    verify(c2H,c3H,c4H,c5H,cTD,cJH,cKD, 5<<28|0x020F<<13|0x0000);
    verify(cKH,c3H,c4H,c5H,cAH,cJD,cKD, 5<<28|0x180E<<13|0x0000);
    verify(cKD,c3H,c3D,c5D,cAD,cJD,cKS, 5<<28|0x1A0A<<13|0x0000); 
    verify(c3D,c4D,c6D,c3H,c9D,c3C,cTD, 5<<28|0x0196<<13|0x0000);
    verify(c3D,c4D,c6D,c3H,c9D,c2D,cTD, 5<<28|0x0196<<13|0x0000);
    verify(c3D,c4D,c6D,c7D,c9D,cJD,cTD, 5<<28|0x03B0<<13|0x0000);
    
    //fh
    verify(c3D,c4D,c3H,c4H,c9S,c3C,cTD, 6<<28|0x0002<<13|0x0004);
    verify(c4S,c4D,c6H,c4H,c3S,c3C,cTD, 6<<28|0x0004<<13|0x0002);
    verify(c3D,c9D,c9H,c3H,c9S,c3C,cTD, 6<<28|0x0080<<13|0x0002);
    
    //quad
    verify(c3D,c9D,c9H,c3H,c3S,c3C,cTD, 7<<28|0x0002<<13|0x0100);
   
    //straight flush low
    verify(cAH,c2H,c3H,c4H,c5H,c9D,cTD, 9<<28|0x0008<<13|0x0000); 
    verify(cAD,c2H,c3H,cJD,cKD,cQD,cTD, 9<<28|0x1000<<13|0x0000); 
    verify(c9D,c7D,c8D,cJD,cKD,cQD,cTD, 9<<28|0x0800<<13|0x0000); 
    printf("\n");
    return 0;
    // initialize the deck
    init_deck( deck );
#if NCARDS == 7
    // loop over every possible seven-card hand
    for(y=0;y<46;y++)
    {
        hand[5] = deck[y];
#if NCARDS >= 6 
        for(z=y+1;z<47;z++)
        {
            hand[6] = deck[z];
#if NCARDS >= 5
		//loop over every possible 5 card hand
            for(a=z+1;a<48;a++)
            {
                hand[0] = deck[a];
                for(b=a+1;b<49;b++)
		        {
			        hand[1] = deck[b];
			        for(c=b+1;c<50;c++)
			        {
                        hand[2] = deck[c];
				        for(d=c+1;d<51;d++)
                        {
                            hand[3] = deck[d];
                            for(e=d+1;e<52;e++)
				            {
                                hand[4] = deck[e];
                                i = eval_hand( hand, NCARDS );
                                j = hand_rank(i);
                                freq[j]++;
                            }
                        }
                    }
                }
            }
#endif
        }
#endif
    }
#endif
    for(i=0;i<=9;i++)
	    printf( "%15s: %8jd\n", value_str[i], freq[i]);
}

/*
Expected Output
---------------
      High Card: 23294460
       One Pair: 58627800
       Two Pair: 31433400
Three of a Kind:  6461620
       Straight:  6180020
          Flush:  4047644
     Full House:  3473184
 Four of a Kind:   224848
 Straight Flush:    41584

see: https://en.wikipedia.org/wiki/Poker_probability#Frequency_of_7-card_poker_hands

*/
