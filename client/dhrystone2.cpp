/*
*************************************************************************
*
*                   "DHRYSTONE" Benchmark Program
*                   -----------------------------
*
*  Version:    C, Version 2.1
*
*  File:       dhry.h (part 1 of 3)
*
*  Date:       May 25, 1988
*
*  Author:     Reinhold P. Weicker
*              Siemens Nixdorf Inf. Syst.
*              STM OS 32
*              Otto-Hahn-Ring 6
*              W-8000 Muenchen 83
*              Germany
*                      Phone:    [+49]-89-636-42436
*                                (8-17 Central European Time)
*                      UUCP:     weicker@ztivax.uucp@unido.uucp
*                      Internet: weicker@ztivax.siemens.com
*
*              Original Version (in Ada) published in
*              "Communications of the ACM" vol. 27., no. 10 (Oct. 1984),
*              pp. 1013 - 1030, together with the statistics
*              on which the distribution of statements etc. is based.
*
*              In this C version, the following C library functions are
*              used:
*              - strcpy, strcmp (inside the measurement loop)
*              - printf, scanf (outside the measurement loop)
*
*/

// Slightly modified for BOINC.
// e.g.: no global variables, since we run this multithreaded

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#ifndef _WIN32
#include <string.h>
#endif

#include "dhrystone.h"

extern bool Func_3(Enumeration EnumParIn);

void Proc_6(DS_DATA& dd, Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par)
{
  *Enum_Ref_Par = Enum_Val_Par;
  if (! Func_3 (Enum_Val_Par))
    /* then, not executed */
    *Enum_Ref_Par = Ident_4;
  switch (Enum_Val_Par)
  {
    case Ident_1:
      *Enum_Ref_Par = Ident_1;
      break;
    case Ident_2:
      if (Int_Glob > 100)
        /* then */
      *Enum_Ref_Par = Ident_1;
      else *Enum_Ref_Par = Ident_4;
      break;
    case Ident_3: /* executed */
      *Enum_Ref_Par = Ident_2;
      break;
    case Ident_4: break;
    case Ident_5:
      *Enum_Ref_Par = Ident_3;
      break;
  } /* switch */
}

void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val, One_Fifty *Int_Par_Ref)
{
  One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 2;
  *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
}

void Proc_8(DS_DATA& dd,
        Arr_1_Dim       Arr_1_Par_Ref,
        Arr_2_Dim       Arr_2_Par_Ref,
        int             Int_1_Par_Val,
        int             Int_2_Par_Val
) {
  REG One_Fifty Int_Index;
  REG One_Fifty Int_Loc;

  Int_Loc = Int_1_Par_Val + 5;
  Arr_1_Par_Ref [Int_Loc] = Int_2_Par_Val;
  Arr_1_Par_Ref [Int_Loc+1] = Arr_1_Par_Ref [Int_Loc];
  Arr_1_Par_Ref [Int_Loc+30] = Int_Loc;
  for (Int_Index = Int_Loc; Int_Index <= Int_Loc+1; ++Int_Index)
    Arr_2_Par_Ref [Int_Loc] [Int_Index] = Int_Loc;
  Arr_2_Par_Ref [Int_Loc] [Int_Loc-1] += 1;
  Arr_2_Par_Ref [Int_Loc+20] [Int_Loc] = Arr_1_Par_Ref [Int_Loc];
  Int_Glob = 5;
}

Enumeration Func_1(
        DS_DATA& dd,
        Capital_Letter   Ch_1_Par_Val,
        Capital_Letter   Ch_2_Par_Val
) {
  Capital_Letter        Ch_1_Loc;
  Capital_Letter        Ch_2_Loc;

  Ch_1_Loc = Ch_1_Par_Val;
  Ch_2_Loc = Ch_1_Loc;
  if (Ch_2_Loc != Ch_2_Par_Val)
    /* then, executed */
    return (Ident_1);
  else  /* not executed */
  {
    Ch_1_Glob = Ch_1_Loc;
    return (Ident_2);
   }
}

bool Func_2(DS_DATA& dd, Str_30  Str_1_Par_Ref, Str_30  Str_2_Par_Ref) {
  REG One_Thirty        Int_Loc;
      Capital_Letter    Ch_Loc=0;

  Int_Loc = 2;
  while (Int_Loc <= 2) /* loop body executed once */
    if (Func_1 (dd, Str_1_Par_Ref[Int_Loc],
                Str_2_Par_Ref[Int_Loc+1]) == Ident_1)
      /* then, executed */
    {
      Ch_Loc = 'A';
      Int_Loc += 1;
    } /* if, while */
  if (Ch_Loc >= 'W' && Ch_Loc < 'Z')
    /* then, not executed */
    Int_Loc = 7;
  if (Ch_Loc == 'R')
    /* then, not executed */
    return (true);
  else /* executed */
  {
    if (strcmp (Str_1_Par_Ref, Str_2_Par_Ref) > 0)
      /* then, not executed */
    {
      Int_Loc += 7;
      Int_Glob = Int_Loc;
      return (true);
    }
    else /* executed */
      return (false);
  } /* if Ch_Loc */
}

bool Func_3(Enumeration Enum_Par_Val)
{
  Enumeration Enum_Loc;

  Enum_Loc = Enum_Par_Val;
  if (Enum_Loc == Ident_3)
    /* then, executed */
    return (true);
  else /* not executed */
    return (false);
}

