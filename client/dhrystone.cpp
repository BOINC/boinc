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
#include <cstdlib>
#include <cstdio>
#include <cstring>
#endif

#include "util.h"
#include "cpu_benchmark.h"
#include "dhrystone.h"

#define structassign(d, s)      d = s


#define TRUE            1
#define FALSE           0


/*
 * Package 1
 */


void Proc_0();
void Proc_1(DS_DATA&, Rec_Pointer PtrParIn);
void Proc_2(DS_DATA&, One_Fifty *IntParIO);
void Proc_3(DS_DATA&, Rec_Pointer *PtrParOut);
void Proc_4(DS_DATA&);
void Proc_5(DS_DATA&);
void Proc_6(DS_DATA&, Enumeration EnumParIn, Enumeration *EnumParOut);
void Proc_7(One_Fifty IntParI1, One_Fifty IntParI2, One_Fifty *IntParOut);
void Proc_8(DS_DATA&, Arr_1_Dim Array1Par, Arr_2_Dim Array2Par, One_Fifty IntParI1, One_Fifty IntParI2);
Enumeration  Func_1(DS_DATA&, Capital_Letter , Capital_Letter );
bool      Func_2(DS_DATA&, Str_30 , Str_30 );
bool Func_3(Enumeration EnumParIn);

int dhrystone(
   double& Vax_Mips, double& int_loops, double& int_time, double min_cpu_time
){
        One_Fifty       Int_1_Loc;
  REG   One_Fifty       Int_2_Loc;
        One_Fifty       Int_3_Loc=0;
  REG   char            Ch_Index;
        Enumeration     Enum_Loc;
        Str_30          Str_1_Loc;
        Str_30          Str_2_Loc;

    unsigned long         Loops;
    DS_DATA dd;

    double                  startclock = 0.0, endclock = 0.0;
    double                  benchtime;
    double Dhrystones_Per_Second;

    REG unsigned long  Run_Index;


  Next_Ptr_Glob = (Rec_Pointer) malloc (sizeof (Rec_Type));
  Ptr_Glob = (Rec_Pointer) malloc (sizeof (Rec_Type));

  Ptr_Glob->Ptr_Comp                    = Next_Ptr_Glob;
  Ptr_Glob->Discr                       = Ident_1;
  Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
  Ptr_Glob->variant.var_1.Int_Comp      = 40;
  strcpy (Ptr_Glob->variant.var_1.Str_Comp,
          "DHRYSTONE PROGRAM, SOME STRING");
  strcpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

  Arr_2_Glob [8][7] = 10;

    Loops = 16000;       // determines runtime
        // This must be small enough that, on the slowest CPU,
        // one loop completes within 10 seconds.


    benchmark_wait_to_start(BM_TYPE_INT);

    boinc_calling_thread_cpu_time(startclock);
    int bigloops = 0;

    do
    {
        for (Run_Index = 0; Run_Index < Loops; ++Run_Index)
        {
            Proc_5(dd);
            Proc_4(dd);
            Int_1_Loc = 2;
            Int_2_Loc = 3;
            strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
            Enum_Loc = Ident_2;
            Bool_Glob = ! Func_2 (dd, Str_1_Loc, Str_2_Loc);
            while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
            {
              Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
              Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
              Int_1_Loc += 1;
            } /* while */
            Proc_8 (dd, Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
            Proc_1 (dd, Ptr_Glob);
            for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
            {
              if (Enum_Loc == Func_1 (dd, Ch_Index, 'C'))
                {
                Proc_6 (dd, Ident_1, &Enum_Loc);
                strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
                Int_2_Loc = Run_Index;
                Int_Glob = Run_Index;
                }
            }
            Int_2_Loc = Int_2_Loc * Int_1_Loc;
            Int_1_Loc = Int_2_Loc / Int_3_Loc;
            Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
            Proc_2 (dd, &Int_1_Loc);
        }
        bigloops++;
    }
    while (!benchmark_time_to_stop(BM_TYPE_INT));

    Loops *= bigloops;

    boinc_calling_thread_cpu_time(endclock);
    benchtime = endclock - startclock;
    int_time = benchtime;
    if (benchtime < min_cpu_time) return -1;

    //printf ("%12.0f runs %6.2f seconds \n",(double) Loops, benchtime);

    Dhrystones_Per_Second = (double) Loops / benchtime;
    Vax_Mips = Dhrystones_Per_Second / 1757.0;
    int_loops = Loops;
#if 0
    printf ("Dhrystones per Second:                      ");
    printf ("%10.0lf \n", Dhrystones_Per_Second);
    printf ("VAX  MIPS rating =                          ");
    printf ("%12.2lf \n",Vax_Mips);
#endif
    free(Next_Ptr_Glob);
    free(Ptr_Glob);
    return 0;
}

void Proc_1(DS_DATA& dd, REG Rec_Pointer Ptr_Val_Par)
{
  REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
                                        /* == Ptr_Glob_Next */
  /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
  /* corresponds to "rename" in Ada, "with" in Pascal           */

  structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
  Ptr_Val_Par->variant.var_1.Int_Comp = 5;
  Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
  Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
  Proc_3 (dd, &Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
  if (Next_Record->Discr == Ident_1)
    /* then, executed */
  {
    Next_Record->variant.var_1.Int_Comp = 6;
    Proc_6 (dd, Ptr_Val_Par->variant.var_1.Enum_Comp,
           &Next_Record->variant.var_1.Enum_Comp);
    Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
    Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
           &Next_Record->variant.var_1.Int_Comp);
  }
  else /* not executed */
    structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
}

void Proc_2(DS_DATA& dd, One_Fifty *Int_Par_Ref)
{
  One_Fifty  Int_Loc;
  Enumeration   Enum_Loc=Ident_1;

  Int_Loc = *Int_Par_Ref + 10;
  do /* executed once */
    if (Ch_1_Glob == 'A')
      /* then, executed */
    {
      Int_Loc -= 1;
      *Int_Par_Ref = Int_Loc - Int_Glob;
      Enum_Loc = Ident_1;
    } /* if */
  while (Enum_Loc != Ident_1); /* true */
}

void Proc_3(DS_DATA& dd, Rec_Pointer *Ptr_Ref_Par)
{
  if (Ptr_Glob != NULL)
    /* then, executed */
    *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
}

void Proc_4(DS_DATA& dd)
{
  bool Bool_Loc;

  Bool_Loc = Ch_1_Glob == 'A';
  Bool_Glob = Bool_Loc | Bool_Glob;
  Ch_2_Glob = 'B';
}

void Proc_5(DS_DATA& dd)
{
        Ch_1_Glob = 'A';
        Bool_Glob = FALSE;
}

