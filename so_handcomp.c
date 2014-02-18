#define L(n)for(i=0;i<n;i++)
#define U unsigned
#define K else if
#define c(a)for(X=a,i=C=0;X;X/=4)C|=(X&1)<<i++;
U g[5],h[5],C,i,X,r[3];U E(U h[]){
U e=0x55555540,k=h[3],a=*h+h[1]+h[2]+h[4]-(k&-16L),o=2*e&a,t=0,v;e&=a;
if(v=e&o/2){t=7;k^=v;while(i=k&k-1)k=i;}K(v=o&o-1){t=6;v/=2;k=o/2^v;}
K(e&&o){t=6;v=o/2;k=(i=e&e-1)?i:e;}else{L(4){X=1<<i;C=((h[X&7]-(k&X))/X&7)&7;
if(C>3){k=h[X&7];t=5;break;}}k&=-64;v=k|k>>26&16;L(4)v&=v*4;if(v){
t+=4;k=v&=~(v/4);}K(i=t)while(C-->4)v=k&=k-1;K(v=o/2)t=3;K(e){o=e&e-1;
v=(i=o&o-1)?o:e;t=1+(o>0);}k^=v;k&=k-1;k&=k-(i==0);}c(v)v=C/8;c(k)
return t<<28|v<<13|C/8;}
#define P(c)C=1<<2*(strchr(S,*c++)-S-1)|1<<strchr(S,*c++)-S;c++;
#define A(h,C)h[C&7]+=C;h[3]|=C;
char*S="CDHS23456789TJQKA",b[32],*c;
main(d){for(;c=gets(b);r[!d?2:d<0]++){L(2){P(c)A(g,C)}L(2){P(c)A(h,C)}
L(5){P(c)A(g,C)A(h,C)}d=E(g)-E(h);L(5)g[i]=h[i]=0;
}L(3)printf("%c:%d\n","12D"[i],r[i]);}
