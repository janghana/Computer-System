/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * Name : jang han
 * Student Number : 201902745
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */

 // 비트 연산을 위해 tmp_x, tmp_y 값을 다 31비트 만큼 shift 시킨다. 그리고 해당 값을 합쳐서 계산한다.
 // tmp_x, tmp_y가 둘 다 0이고 합친 값 sum이 1이면 오버플로우, 둘 다 1이고 합친값이 0이면 오버플로우이다.
 /*
 * *Explanation* *
 * To calculate bit arithmetic, we can shift x and y in tmp_x and tmp_y.
 * We can sum the tmp_x and tmp_y in 'sum'.
 * So, we determin the value for return value in case1, 2.
 * The case1 is tmp_x and tmp_y are zero and sum is one and The case2 is tmp_x and tmp_y are one and sum is zero value.
 * We can return the value for these cases.
 */
int addOK(int x, int y) {
  int tmp_x = x >> 31;
  int tmp_y = y >> 31;
  int sum = ( x + y ) >> 31;
  return !(((~sum)&tmp_x&tmp_y) | (sum&(~tmp_x)&(~tmp_y)));
}

/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */

 // tmp_x에 10101010..101010의 값을 넣고 이 값과 parameter x 값을 & 연산자로 계산하여 홀수 있는 값만 가져온다.
 // 그리고 홀수 1010..1010 값과 xor 연산자를 통해서 1인 값을 다 비교하여 다 1인 경우를 고려한다.
 /*
 * *Explanation* *
 * We can result the value that is obtained by adding the 0xAA and 0xAA << 8 and ... 0xAA << 24.
 * This means that 1010 and 1010 .. and 1010.
 * We can calculate the value that x & operations tmp_x value is operated by nor operation with tmp_x.
 */
int allOddBits(int x) {
   int tmp_x = 0xAA | 0xAA << 8 | 0xAA << 16 | 0xAA << 24;
   return !((tmp_x&x) ^ tmp_x);
}
/* 
 * bitNor - ~(x|y) using only ~ and & 
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1

 */
 
 // 드모르간 법칙을 이용하여 비트 nor를 구현하였다.
 /*
 * *Explanation* *
 * We can return the value that is used by De Morgan's laws
 */
int bitNor(int x, int y) {
  return (~x) & (~y);
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
 // no_NaN에 0x80000000의 값을 xor 연산자를 통해 uf와 계산하여 sign bit의 값을 바꾼다.
 // 그리고, exp의 값을 cu에 저장한다. cu가 모두 1이고 frac가 모두 0이 아니라면 NaN이다.
 // 그 외 경우 no_NaN으로 negative 값을 리턴한다.

/*

 * *Explanation* *
 * We can define "no_NaN" that uf is negative value which uf is calculated with xor operations with 0x80000000.
 * And exp value saves in "cu". If exp value is all 1 and frac value is not zero value, we can determin that uf is NaN.
 * Otherwise, we can return the value that no_NaN is negative.
 
 */
unsigned float_neg(unsigned uf) {

	unsigned no_NaN = ((0x80 << 24) ^ uf);
	unsigned cu = 0xff << 23;

	if(((cu & uf) == cu) && (uf & ((1<<23) + (~0)))) {return uf;}
	return no_NaN;
}

/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */

/*
2의 제곱수인 e와 frac를 담을 변수를 선언한다. 그리고 e_23을 선언하여 exp의 부분만을 남겨놓는다.
첫 번째 경우, NaN의 경우를 고려할 수 있다. 그리고, 두 번째 경우, exp가 모두 1인 경우를 고려하여 NaN인
경우를 고려할 수 있다. 세 번째로는 exp가 모두 0인 경우를 고려할 수 있다. 이 경우, return 값을
uf를 shift 연산으로 1씩 밀은 것과 uf에 1의 값을 31 shift한 값을 or하여 return한다.
또 다른 마지막 경우는 exp 값에 1을 더하여 uf와 더하여 마무리한다.
*/
/*
 * *Explanation* *
 * The first case is the NaN case or uf is negative value.
 * We can consider that the first case is e == 0 and frac == 0 or e == 0 and
 * frac != 0x00000000.
 * The second case is that uf is very small value. 
 * The third case is that exp is all zero.
 * This case, we can return the value that is or operation by uf is shifted left by 1
 * and 1 is shifted by 31.
 * Otherwise, we can return the value that is plused by uf and exp + 1
 */
unsigned float_twice(unsigned uf) {

   unsigned e = uf & (0x7f << 24 | 0x80 << 16);
   unsigned e_23 = e >> 23;
   unsigned frac = uf & (0x7f << 16 | 0xff << 8 | 0xff);

   // first
   if (uf == (0x80 << 24) || uf == 0){
      if (frac == (0x00)){ return uf; }
      else if (frac != (0x00)){ return uf; }
      else {
         return uf;
      }
   }
   // second
   else if ((e_23 & 0xff) == 0xff){
      return uf;
   }
   // third
   else if ((e_23 & 0xff) == 0x00){
		return (uf & (1<<31)) | (uf<<1);
   }
	//Otherwise.
   else{
      return uf + (1<<23);
   }
}
/* 
 * rempwr2 - Compute x%(2^n), for 0 <= n <= 30
 *   Negative arguments should yield negative remainders
 *   Examples: rempwr2(15,2) = 3, rempwr2(-35,3) = -3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */

 // pwr2n에 bit가 모두 1인 값을 n만큼 왼쪽으로 shift한다. 그리고 그 값을 다시 ~ operation을 취하면 2^n - 1의 값을 구현할 수 있다.
 // x의 sign 비트를 ms에 저장한다. 그리고 result에 x와 2^n - 1의 값의 and 연산을 저장한다.
 // case1) 만약, result의 값이 0이라면 << n 연산을 취해서 1을 더하면 1의 bit 값을 가지고 ms의 값과 연산하게 된다. 그러면 부호를 결정할 수 있다. 그렇다면 값은 1이나 -1을 가지게 된다.
 // 이 값에다가 reulst의 값을 or operation하면 x%(2^n)의 값이 나오게된다.
 // case2) result의 값이 0이 아니라면 !!result의 값은 1이 되므로 1을 n만큼 left shift하고 
 // 여기에 1을 더하면 2^n에 1을 더한 값과 ms를 and 연산하여 부호를 결정할 수 있다. 그렇다면 2^n 이나 -2^n이 된다.
 // 이 값을 result와 or operation하면 x%(2^n)의 값이 나오개 된다.

 /*
 * *Explanation* *
 * We can define 2^n - 1 calculation in valuable 'pwr2n'. The value that all of the bits are 1 is shifted by n value.
 * And that value is operated ~ operation. And x's sign bit stores in ms. And x and pwr2n are calculated 'and' operation in 'result' value.d
 */
int rempwr2(int x, int n) {
   int pwr2n = ~(~0 << n);
   int ms = x >> 31;
   int result = x & pwr2n;
   return result | (((~((!(!result)) << n)) + 1) & ms);
}
