char*S="CDHS23456789TJQKA",b[32],*c;
unsigned h[5];
main(int C){
  c=gets(b);
  while(*c){C=1<<2*(strchr(S,*c++)-S-1)|1<<strchr(S,*c++)-S;c++;h[C&7]+=C;h[3]|=C;}
  printf("%08x\n",E(h));
}
