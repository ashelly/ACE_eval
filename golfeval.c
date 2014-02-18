#define L(n)for(i=0;i<n;i++)
#define U unsigned
#define K else if
#define c(a)for(X=a,i=C=0;X;X/=4)C|=(X&1)<<i++;
U C,i,X,h[5];U E(U h[]){U e=0x55555540,k=h[3],a=*h+h[1]+h[2]+h[4]-(k&-16L),o=2*e&a,
t=0,v;e&=a;if(v=e&o/2){t=7;k^=v;while(i=k&k-1)k=i;}K(v=o&o-1){t=6;v/=2;k=o/2^v;
}K(e&&o){t=6;v=o/2;k=(i=e&e-1)?i:e;}else{L(4)if((C=((a=h[(1<<i)&7])>>i)&7)>4)
{k=a;t=5;break;}k&=-64;v=k|k>>26&16;L(4)v&=v*4;if(v){t+=4;k=v&=~(v/4);}K(i=t)
while((v=k)&&C-->5)k&=k-1;K(v=o/2)t=3;K(e){o=e&e-1;v=(i=o&o-1)?o:e;t=1+(o>0);}
k^=v;k&=k-1;k&=k-(i==0);}c(v)v=C/8;c(k)return t<<28|v<<13|C/8;}
char*S="CDHS23456789TJQKA",b[32],*c;
main(){c=gets(b);while(*c){C=1<<2*(strchr(S,*c++)-S-1)|1<<strchr(S,*c++)-S;
c++;h[C&7]+=C;h[3]|=C;}printf("%08x\n",E(h));}
