#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//: Based on code collected in the XPokerEval library at:
// http://www.codingthewheel.com/archives/poker-hand-evaluator-roundup
//: original code from Ray Wouten, Steve Brecher, and others on twoplustwo.com  


typedef int64_t LARGE_INTEGER;
typedef struct timespec sysTime_t;

#include "eval.h"

#define MS_PER_SEC 1000.0f
#define LOTS  100000000 //1e6
#define hand_rank(r)       ((r)>>28)



struct timespec timings,endtimings;

const char HandRanks[][16] = {"High uint64_t","Pair","Two Pair","Three of a Kind","Straight","Flush","Full House","Four of a Kind","BAD","Straight Flush"};


//"Knuth Shuffle"  or "Fisher-Yates"
void Shuffle(uint64_t *Deck)
{
  int r,i;
	uint64_t temp;
    for(i=51;i>=0;i--)
    {
        r=(int)(((float)rand())/RAND_MAX*i);
		temp = Deck[i];
		Deck[i]=Deck[r];
		Deck[r]=temp;
    }
}


//** Linux specific timing **/
sysTime_t platformTimeElapsed(sysTime_t now, sysTime_t then)
{
  sysTime_t e;
  e.tv_sec = now.tv_sec-then.tv_sec;
  if (now.tv_nsec < then.tv_nsec){
	 e.tv_sec-=1;
	 e.tv_nsec = now.tv_nsec+(1e9-then.tv_nsec);
  }
  else{
	 e.tv_nsec = now.tv_nsec-then.tv_nsec;
  }
  return e;
}
double platformSysTimeToMs(sysTime_t timeIn) {
    return (double)timeIn.tv_sec* MS_PER_SEC + (double)timeIn.tv_nsec / 1e6;
}

//Eval LOTS of random hands
uint64_t hands[LOTS][5]={0};


int main(int argc, char*argv[])
{
	long i;
	uint64_t Deck[52];
	int cardsLeft = 52;

	int count = 0;
	int handTypeSum[10]={0};

	srand(argc);

	for (i=0;i<52;i++)
	{
		Deck[i]=makecard(i);
	}
	Shuffle(Deck);

	
	for (i=0;i<LOTS;i++)
	{
		if (cardsLeft<7)
		{
			Shuffle(Deck);
			cardsLeft=52;
		}
		memset(hands[i],0,sizeof(uint64_t)*5);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
		--cardsLeft;
		addcard(hands[i],Deck[cardsLeft]);
	}


//TIME LOTS of Evals
	count = 0;
	clock_t timer = clock();						    // start regular clock
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timings); //and h-p clock

	for (i=0;i<LOTS;i++)
	{
	  uint64_t r = evaluate( hands[i] );
	  handTypeSum[hand_rank(r)]++;
	  count++;
	}

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &endtimings);	  // end the high precision clock
	timer = clock() - timer;				  // end the regular clock

	for (i = 0; i <= 9; i++)			  // display results
	  printf("\n%16s = %d", HandRanks[i], handTypeSum[i]);

	printf("\nTotal Hands = %d\n", count);

	double clocksused = platformSysTimeToMs(platformTimeElapsed(endtimings,timings));

	// and display the clock results
	printf("\nValidation seconds = %.4lf\nTotal HighPrecision Clocks = %lf\nHighPrecision clocks per lookup = %lf\n", 
			 (double)timer/CLOCKS_PER_SEC, clocksused, (double) clocksused /  count) ;
    printf("\n(smaller is better...\n");

	 printf("\n %lf Mhands/sec\n",count/((double)clocksused/1000)/1000000.0);

}
