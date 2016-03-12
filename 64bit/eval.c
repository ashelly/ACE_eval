#include <stdio.h>
#include <stdint.h>
#include "eval.h"

/* Card representation:
 * 4 values for each suit
 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
 *      A... K... Q... J... T... 9... 8... 7... 6... 5... 4... 3... 2... Ext. Mask
 * (Ext. reserved to copy A bit in straight detection)
 * 1 value to save all card ranks
 *
 * Detection:
 *   9 Straight Flush:
 *     Mask == 5
 *     Value of that suit forming a straight.
 *   7 Four of a Kind:
 *     Sum of values equal 4.
 *   6 Full House:
 *     Sum of values have 3 and 2 (or 3 and 3).
 *   5 Flush:
 *     Mask == 5
 *   4 Straight:
 *     Value with card ranks (hand[3]) have 5 bits.
 *   3 Three of a Kind:
 *     Sum of values have 3.
 *   2 Two Pair:
 *     Sum of values have 2 and 2 (and 2).
 *   1 One pair:
 *     Sum of values have 2.
 *   0 High card:
 */

inline uint64_t makecard(int i) {return (uint64_t)1 << (4 * (i % 13) + 8) | 1 << (i / 13);}

inline void addcard(uint64_t hand[5], uint64_t card) {
    hand[card & 0x7] += (card & -16) + 1;
    hand[3] |= card;
}

/*	Compress algorithm
 	000a 000b 000c 000d 000e 000f 000g 000h 000i 000j 000k 000l 000m 000n 000o 000p 	x
|	0000 00a0 00b0 00c0 00d0 00e0 00f0 00g0 00h0 00i0 00j0 00k0 00l0 00m0 00n0 00o0     x >> 3
    -------------------------------------------------------------------------------
=	000a 00ab 00bc 00cd 00de 00ef 00fg 00gh 00hi 00ij 00jk 00kl 00lm 00mn 00no 00op		x | (x >> 3)
&	0000 0011 0000 0011 0000 0011 0000 0011 0000 0011 0000 0011 0000 0011 0000 0011		0x0303030303030303
	-------------------------------------------------------------------------------
=	0000 00ab 0000 00cd 0000 00ef 0000 00gh 0000 00ij 0000 00kl 0000 00mn 0000 00op		(x | (x >> 1) & 0x0303030303030303
|	0000 0000 0000 ab00 0000 cd00 0000 ef00 0000 gh00 0000 ij00 0000 kl00 0000 mn00		x >> 6
	-------------------------------------------------------------------------------
=	0000 00ab 0000 abcd 0000 cdef 0000 efgh 0000 ghij 0000 ijkl 0000 klmn 0000 mnop		x | (x >> 6)
&	0000 0000 0000 1111 0000 0000 0000 1111 0000 0000 0000 1111 0000 0000 0000 1111		0x000f000f000f000f
	-------------------------------------------------------------------------------
=	0000 0000 0000 abcd 0000 0000 0000 efgh 0000 0000 0000 ijkl 0000 0000 0000 mnop		(x | (x >> 6) & 0x000f000f000f000f
|	0000 0000 0000 0000 0000 0000 abcd 0000 0000 0000 efgh 0000 0000 0000 ijkl 0000		x >> 12
	-------------------------------------------------------------------------------
	0000 0000 0000 abcd 0000 0000 abcd efgh 0000 0000 efgh ijkl 0000 0000 ijkl mnop		x | (x >> 12)
&	0000 0000 0000 0000 0000 0000 1111 1111 0000 0000 0000 0000 0000 0000 1111 1111		0x000000ff000000ff	
	-------------------------------------------------------------------------------
=	0000 0000 0000 0000 0000 0000 abcd efgh 0000 0000 0000 0000 0000 0000 ijkl mnop		(x | (x >> 12) & 0x000000ff000000ff
|	0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 abcd efgh 0000 0000		x >> 24
	-------------------------------------------------------------------------------
=	0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 abcd efgh ijkl mnop		x | (x >> 24)
&   0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1111 1111 1111 1111     0x000000000000ffff
    */
inline uint32_t compress(uint64_t card) {
    card = (card | (card >> 3)) & 0x0303030303030303;
    card = (card | (card >> 6)) & 0x000f000f000f000f;
    card = (card | (card >> 12)) & 0x000000ff000000ff;
    card = (card | (card >> 24)) & 0x000000000000ffff;
    return (uint32_t)card>>2;
}

int i;
uint32_t evaluate(uint64_t hand[]) {
    uint64_t value, kicker = hand[3], result = 0, temp;
    uint64_t count = hand[0] + hand[1] + hand[2] + hand[4];

    //Look for Four of a Kind
    if((value = count & 0x0444444444444400)) {
        value >>= 2; //aligning
        kicker = hand[3] ^ value;
        while((temp = kicker & (kicker - 1)))
            kicker = temp;
        return 7 << 28 | compress(value) << 13 | compress(kicker);
    }


    //first find all pair and better bits count&0x0222222222222200, after check the right bit, if 1 - we have set
    uint64_t pair = count & 0x0222222222222200;
    uint64_t set = count & pair >> 1;
    //exclude sets from pair
    pair ^= set << 1;
    pair >>= 1; //aligning


    //Full House
    //catch 2 sets
    if((value = set & (set - 1))) {
        kicker=set ^ value;
        return 6<<28|compress(value)<<13|compress(kicker);
    }
    //catch set plus one or two pairs
    //It clears one bit from the pairs field if needed when setting kicker
    //(since AAAKKQQ ranks the same as AAAKKQJ)
    if(set && pair) {
        value = set;
        temp = pair & (pair - 1);
        kicker = (temp) ? temp : pair;
	    return 6<<28|compress(value)<<13|compress(kicker);
    }

    //Look for Flush
    for(i = 0; i < 4; i++) {
        int index = (1 << i) & 7;
        if((hand[index] & 0xF) >= 5) {
            result = 5;
            kicker = hand[index];
            count = kicker & 0x7; 
            break;
        }
    }
    //Look for Straight
    //clear the suit bits
    //count = kicker & 0x7; 
    kicker &= -16; 
    //copy ace bit to Ext., to catch A,2,3,4,5 straight
    value = kicker|((kicker>>52)&0x10);
    //the next line zeros value unless there are at least 5 cards in a row.  
    //for a 6 or 7 card straight, there will be multiple consecutive bits set in value: 
    //    `value&=~(value/16)` clears all but the highest. 
    value &= value << 4;
    value &= value << 4;
    value &= value << 4;
    value &= value << 4;
    
    if(value) {
         result += 4;
         value &= ~(value / 16);
         return result<<28|compress(value)<<13;
    }
    //finish up the flush processing: 'result' is only set for flush, 
    //store the high 5 cards in `kicker` and `value` 
    else if(result) {
        while(count-->5) {
            kicker &= kicker - 1;
        }
        return result<<28|compress(kicker)<<13;
    }
    
    //Look for Three of a Kind
    else if((value = set)) {
        kicker^=value;
        kicker&=kicker-1;
        kicker&=kicker-1;
        return 3<<28|compress(value)<<13|compress(kicker);
    }
    //Look for Pair
    //we might have 1,2, or 3 of them.
    //'temp' will be set if there is more than one, `temp2` will be set if there are 3. 
    //'result' is 1 or 2.
    else if(pair) {
        temp = pair & (pair - 1);
        if(temp & (temp - 1)) {
            kicker ^= temp;
            kicker&=kicker-1;
            return 2<<28|compress(temp)<<13|compress(kicker);
        }
        else {
            kicker ^= pair;
            kicker&=kicker-1;
            kicker&=kicker-1;
            return (1+(temp>0))<<28|compress(pair)<<13|compress(kicker);
        }

    }
    //Final
    //4 bits for the type 0..9, 13 bits for the value cards, 13 for the kicker.
    kicker&=kicker-1;
    kicker&=kicker-1;
            
    return 0|0<<13|compress(kicker);
}
