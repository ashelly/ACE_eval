char*c,b[64],r;
char*names[]={"Two","Three","Four","Five","Six","Seven","Eight","Nine","Ten","Jack","Queen","King","Ace"};
main(){
  
  unsigned  d,v,t,n=0;
  do{
	 c=gets(b);
	 printf("%s\n",c);
  } while (*c=='#');
  r=*c++;
  sscanf(c,"%x",&v);
  printf("%x\n",v);
  t=26;
  while (t){--t;
	 if(v&(1<<t)){b[n++]=(t)%13;
		printf("found a %d : %d %s\n",t+1,b[n-1],names[b[n-1]]);
	 }
  }
 switch (r){
 case '9': printf("Straight Flush - %s high\n",names[b[0]]);break;
 case '8': printf("Error\n");break;
 case '7': printf("4 of a Kind  %s's over %s\n",names[b[0]],names[b[1]]);break;
 case '6': printf("Full House - %s's over %ss\n",names[b[0]],names[b[1]]);break;
 case '5': printf("Flush - %s with  %s %s %s %s\n",names[b[0]],names[b[1]],names[b[2]],names[b[3]],names[b[4]]);break;
 case '4': printf("Straight - %s high\n",names[b[0]]);break;
 case '3': printf("3 of a Kind  %s's over %s %s\n",names[b[0]],names[b[1]],names[b[2]]);break;
 case '2': printf("2 Pair - %s's and %s's with a %s\n",names[b[0]],names[b[1]],names[b[2]]);break;
 case '1': printf("Pair - %s's over %s %s %s\n",names[b[0]],names[b[1]],names[b[2]],names[b[3]]);break;
 case '0': printf("High Card - %s with  %s %s %s %s\n",names[b[0]],names[b[1]],names[b[2]],names[b[3]],names[b[4]]);break;
 }
}
 
 
 
	
  

