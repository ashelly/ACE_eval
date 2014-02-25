#include <stdio.h>
static const char *names[]= {
  "Two","Three","Four",
  "Five","Six","Seven",
  "Eight","Nine","Ten",
  "Jack","Queen","King","Ace"
};
unsigned cards[5];

#define CARD(i) names[cards[(i)]]

const char* cardi(int i){
  printf("fetching name for %d %d\n",i,cards[(i)]);
  return names[0];
}

int main(){
  
  char buffer[128]; 
  char*c,rank;
  unsigned hand,bit,n=0;

  do {
	 c=fgets(buffer,128,stdin); //read a whole line
	 printf("%s\n",c);
  } while (*c=='#'); //skip comments

  rank=*c++; //extract rank char

  sscanf(c,"%x",&hand); //convert remainder to hex

  for (bit=26;bit-->0;) { //extract bits downto 0
	 if (hand & (1<<bit)) {
		cards[n++]=(bit)%13;  
	 }
  }
  switch (rank){
  case '9': printf("Straight Flush, %s high\n",CARD(0));break;
  case '8': printf("Error\n");break;
  case '7': printf("4 of a Kind, %ss with a %s\n",CARD(0),CARD(1));break;
  case '6': printf("Full House, %ss over %ss\n",CARD(0),CARD(1));break;
  case '5': printf("Flush, %s %s %s %s %s\n",CARD(0),CARD(1),CARD(2),CARD(3),CARD(4));break;
  case '4': printf("Straight, %s high\n",CARD(0));break;
  case '3': printf("3 of a Kind, %ss over %s %s\n",CARD(0),CARD(1),CARD(2));break;
  case '2': printf("2 Pair, %ss and %ss with a %s\n",CARD(0),CARD(1),CARD(2));break;
  case '1': printf("Pair, %ss over %s %s %s\n",CARD(0),CARD(1),CARD(2),CARD(3));break;
  case '0': printf("High Card, %s with  %s %s %s %s\n",CARD(0),CARD(1),CARD(2),CARD(3),CARD(4));break;
 }
}
 
 
 
	
  

