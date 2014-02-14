#include <stdio.h>
#include "ace_eval.h"

#define NCARDS 7

// Derived from 'allfive.c' by Kevin Suffecool
// http://suffecool.net/poker/code/allfive.c
/*************************************************/
/*                                               */
/* This code tests an evaluator, by looping over */
/* all 2,598,960 possible five card hands, cal-  */
/* culating each hand's distinct value, and dis- */
/* playing the frequency count of each hand type */
/*                                               */
/* Kevin L. Suffecool, 2001                      */
/* suffecool@bigfoot.com                         */
/*                                               */
/*************************************************/
unsigned int eval_hand( Card *hand, int ncards );
void init_deck( Card *deck );


/* ****************************************
//evaluators under test must provide:
typedef Card;   // a card type.  

int eval_hand( hand, ncards );  

void init_deck( Card *deck ) //
//   This routine initializes the deck.  A deck of cards is
//   simply an array of Cards of length 52 (no jokers).  This
//   routine should populate the array 

int hand_rank(int eval_result);
// returns one of 9 possible values indicating rank

char* value_str[]; //when indexed by hand_rank, gives rank name
*/


//Adapters for minieval
void init_deck(Card* deck) {
  int i;
	for (i=0;i<52;i++) deck[i]=ACE_makecard(i);
}

Card eval_hand(Card* hand,int n) {
  int i;
  Card h[5]={0};
  for (i=0;i<n;i++) ACE_addcard(h,hand[i]);
  return ACE_minieval(h);
}
#define hand_rank(r)       ((r)>>28)

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

// *** Test Code ***

main()
{
  Card deck[52], hand[7]={0}, freq[10]={0};
  int a, b, c, d, e, j;
  int y=0,z=-1;
  Card i;
  
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
	 printf( "%15s: %8d\n", value_str[i], freq[i]);
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
