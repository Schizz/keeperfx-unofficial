/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_math.c
 *     Math routines.
 * @par Purpose:
 *     Fast math routines, mostly fixed point.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_math.h"

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned short lbSqrTable[] = 
  {0x0001, 0x0002, 0x0002, 0x0004, 0x0005, 0x0008, 0x000B, 0x0010,
   0x0016, 0x0020, 0x002D, 0x0040, 0x005A, 0x0080, 0x00B5, 0x0100,
   0x016A, 0x0200, 0x02D4, 0x0400, 0x05A8, 0x0800, 0x0B50, 0x1000,
   0x16A0, 0x2000, 0x2D41, 0x4000, 0x5A82, 0x8000, 0xB504, 0xFFFF,};
/******************************************************************************/
#ifndef __cplusplus
#include "bflib_mathinln.h"
#endif
/******************************************************************************/
long __fastcall LbSqrL(long x)
{
  long y;
  if (x <= 0)
    return 0;
  // 
  asm ("bsrl     %1, %%eax;\n"
       "movl %%eax, %0;\n"
       :"=r"(y)  // output
       :"r"(x)   // input
       :"%eax"   // clobbered register
       );
  y = lbSqrTable[y];
  while ((x/y) < y)
    y = ((x/y) + y) >> 1;
  return y;
}

long LbMathOperation(unsigned char opkind, long val1, long val2)
{
  switch (opkind)
  {
    case MOp_EQUAL:
      return val1 == val2;
    case MOp_NOT_EQUAL:
      return val1 != val2;
    case MOp_SMALLER:
      return val1 < val2;
    case MOp_GREATER:
      return val1 > val2;
    case MOp_SMALLER_EQ:
      return val1 <= val2;
    case MOp_GREATER_EQ:
      return val1 >= val2;
    case MOp_LOGIC_AND:
      return val1 && val2;
    case MOp_LOGIC_OR:
      return val1 || val2;
    case MOp_LOGIC_XOR:
      return (val1!=0) ^ (val2!=0);
    case MOp_BITWS_AND:
      return val1 & val2;
    case MOp_BITWS_OR:
      return val1 | val2;
    case MOp_BITWS_XOR:
      return val1 ^ val2;
    case MOp_SUM:
      return val1 + val2;
    case MOp_SUBTRACT:
      return val1 - val2;
    case MOp_MULTIPLY:
      return val1 * val2;
    case MOp_DIVIDE:
      return val1 / val2;
    case MOp_MODULO:
      return val1 % val2;
    default:
      return val1;
  }
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif