/*
 *      "DHRYSTONE" Benchmark Program for PCs
 *
 *      Version:        C/1.1, 
 *
 *      Date:           PROGRAM updated 02 Feb 95, RESULTS updated 03/31/86
 *
 *      Author:         Reinhold P. Weicker, CACM Vol27, No10,10/84 pg.1013
 *                      Translated from ADA by Rick Richardson.
 *                      Translated from C to ANSI C by James Day.
 *
 **************************************************************************
 *
 *                  This version by Roy Longbottom
                      101323.2241@compuserve.com
                            April 1996
 *
 *   A number of enhancements have been included to conform to a format
 *   used for a number of standard benchmarks:
 *
 *  1. Function prototypes are declared and function headers have
 *     embedded parameter types to produce code for C and C++
 *
 *  2. Timing function include (compiler dependent).
 *
 *  3. Additional date function included (compiler dependent).
 *
 *  4. Automatic run time calibration rather than fixed parameters
 *
 *  5. Initial calibration with time display to show linearity
 *     
 *  6. Facilities included for typing in details of system used etc.
 *     The input section can be avoided using a command line
 *     parameter N (for example dhry1nd.exe N).
 *  
 *  7. Compiler details in code in case .exe files used elsewhere
 *
 *  8. Results appended to a text file (Dhry.txt)
 *
 *  Other changes:
 
 *     Proc1 statement Proc3(NextRecord.PtrComp) changed to
 *     Proc3(&NextRecord.PtrComp) as in Dhrystone 2.1 in order to compile.
 *
 *     Loops and loop variable i are both unsigned long to work with 16
 *     bit compilers. The only check that the program has executed properly
 *     is the display of loops executed + 10 (Array2Glob8/7). With a 16 bit
 *     compiler this should still indicate OK but the number will appear
 *     to be incorrect as it is derived from overflowing 16 bit integers
 *     and might have a negative value.
 *
 **************************************************************************
 *
 *                             Timing
 *
 *  The PC timer is updated at about 18 times per second or resolution of
 *  0.05 to 0.06 seconds. Running time is arranged to be greater than five
 *  seconds.
 *
 **************************************************************************
 *
 *                  Example of Output
 *
 *       Dhrystone Benchmark, Version 1.1 (Language: C)
 *
 *       Compiler        Watcom C/ C++ 10.5 Win386 
 *       Optimisation      -otexan -zp8 -5r
 *
 *           10000 runs   0.00 seconds 
 *          100000 runs   0.38 seconds 
 *          200000 runs   0.66 seconds 
 *          400000 runs   1.43 seconds 
 *          800000 runs   2.80 seconds 
 *         1600000 runs   5.55 seconds 
 *
 *       Array2Glob8/7: O.K.     1600010
 *
 *       Microseconds for one run through Dhrystone:         3.47 
 *       Dhrystones per Second:                          288288 
 *       VAX  MIPS rating =                                164.08 
 *
 *     Enter the following data which will be filed with the results
 *
 *     Month run         9/1996                          #
 *     PC model          Escom
 *     CPU               Pentium
 *     Clock MHz         100
 *     Cache             256K
 *     Options           Neptune chipset
 *     OS/DOS            Windows 95
 *     Compiler          Watcom C/C++ 10.5 Win 386       #
 *     OptLevel          -otexan -zp8 -5r                #
 *     Run by            Roy Longbottom
 *     From              UK
 *     Mail              101323.2241@compuserve.com
 *
 *                       Included by program             #
 *
 *   The speed and running details are appended to file Dhry.txt
 *                    
 ************************************************************************
 *
 *                     Examples of Results
 *
 *  Pre-compiled 32 bit codes were produced via a Watcom C/C++ 10.5
 *  compiler. Versions are available for DOS, Windows 3/95 and NT/Win95.
 *  Both non-optimised and optimised programs are available. The latter
 *  has options as in the above example. These include the -otexan default
 *  options which can place functions in-line and carry out loop
 *  optimisation. These are not supposed to be used with this benchmark
 *  but it seems that some published results ignore this directive.
 *
 *  Results produced are not necessarily consistent between runs. Besides
 *  occasional interference from Windows, the timing seems to be dependent
 *  on where the code or data is placed in memory. Representative good
 *  results are:
 *
 *                             32 Bit Operation
 *
 *                              Opt   No Opt                        Version/
 *               MHz   Cache   VaxMips VaxMips Make/Options            Via
 *
 *   AM80386DX    40    128K    17.4     4.5   Clone                  Win/W95
 *   80486DX2     66    128K    42.3    12.6   Escom SIS chipset      Win/W95
 *   80486DX2     66    128K    41.5    13.3   Escom SIS chipset       NT/W95
 *   80486DX2     66    128K    46.3    12.9   Escom SIS chipset      Dos/Dos
 *   Pentium     100    256K   164.1    27.0   Escom Neptune chipset  Win/W95
 *   Pentium     100    256K   157.8    29.7   Escom Neptune chipset   NT/W95 
 *   Pentium     100    256K   176.5    28.1   Escom Neptune chipset  Dos/Dos
 *   Pentium Pro 200    256K   372.8    92.4   Dell XPS Pro200n        NT/NT
 *
 *  The results are as produced when compiled as Dhry1.c. Compiling as
 *  Dhry1.cpp gives similar speeds but the code is slightly different. Other
 *  results were produced for the above 486 and Pentium via a Watcom Windows
 *  16 bit system and Borland Light development system with no optimising
 *  options:
 *
 *                             16 Bit Operation
 *
 *                         Watcom Opt   Watcom No Opt     Borland
 *                          VaxMips        VaxMips        VaxMips
 *
 *      80486DX2  66          38.8           13.7          14.5
 *      Pentium  100          91.5           41.7          44.5
 * 
 ***************************************************************************
 *
 */

// Slightly modified for BOINC

#ifdef _WIN32
#include "stdafx.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

#define structassign(d, s)      d = s

typedef enum    {Ident1, Ident2, Ident3, Ident4, Ident5} Enumeration;

typedef int     OneToThirty;
typedef int     OneToFifty;
typedef char    CapitalLetter;
typedef char    String30[31];
typedef int     Array1Dim[51];
typedef int     Array2Dim[51][51];

struct  Record
{
        struct Record           *PtrComp;
        Enumeration             Discr;
        Enumeration             EnumComp;
        OneToFifty              IntComp;
        String30                StringComp;
};

typedef struct Record   RecordType;
typedef RecordType *    RecordPtr;

#define TRUE            1
#define FALSE           0


void Proc0();
void Proc1(RecordPtr PtrParIn);
void Proc2(OneToFifty *IntParIO);
void Proc3(RecordPtr *PtrParOut);
void Proc4();
void Proc5();
void Proc6(Enumeration EnumParIn, Enumeration *EnumParOut);
void Proc7(OneToFifty IntParI1, OneToFifty IntParI2, OneToFifty *IntParOut);
void Proc8(Array1Dim Array1Par, Array2Dim Array2Par, OneToFifty IntParI1,
                                                OneToFifty IntParI2);
extern Enumeration  Func1(CapitalLetter CharPar1, CapitalLetter CharPar2);
extern bool      Func2(String30 StrParI1, String30 StrParI2);
bool Func3(Enumeration EnumParIn);

 
/*
 * Package 1
 */
int             IntGlob;
bool         BoolGlob;
char            Char1Glob;
char            Char2Glob;
Array1Dim       Array1Glob;
Array2Dim       Array2Glob;
RecordPtr       PtrGlb;
RecordPtr       PtrGlbNext;
int             getinput = 1;


void dhrystone(
   double& Dhrystones_Per_Second, double& Vax_Mips
){
    OneToFifty              IntLoc1;
    OneToFifty              IntLoc2;
    OneToFifty              IntLoc3;
    char                    CharIndex;
    Enumeration             EnumLoc;
    String30                String1Loc;
    String30                String2Loc;
    unsigned long         Loops;
                          
    double                  startclock;
    double                  benchtime;
        
    register unsigned long  i;

 
/***********************************************************************
 *         Change for compiler and optimisation used                   *
 ***********************************************************************/
       
    PtrGlbNext = (RecordPtr) malloc(sizeof(RecordType));
    PtrGlb = (RecordPtr) malloc(sizeof(RecordType));
    PtrGlb->PtrComp = PtrGlbNext;
    PtrGlb->Discr = Ident1;
    PtrGlb->EnumComp = Ident3;
    PtrGlb->IntComp = 40;
    strcpy(PtrGlb->StringComp, "DHRYSTONE PROGRAM, SOME STRING");
    strcpy(String1Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

    //printf ("Dhrystone Benchmark, Version 1.1 (Language: C or C++)\n");
    
    Loops = 16000000;       // determines runtime

   do
     {
       Array2Glob[8][7] = 10;

      /*****************
      -- Start Timer --
      *****************/
      
        startclock = dtime();
        
        for (i = 0; i < Loops; ++i)
        {
                Proc5();
                Proc4();
                IntLoc1 = 2;
                IntLoc2 = 3;
                strcpy(String2Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
                EnumLoc = Ident2;
                BoolGlob = ! Func2(String1Loc, String2Loc);
                while (IntLoc1 < IntLoc2)
                {
                        IntLoc3 = 5 * IntLoc1 - IntLoc2;
                        Proc7(IntLoc1, IntLoc2, &IntLoc3);
                        ++IntLoc1;
                }
                Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3);
                Proc1(PtrGlb);
                for (CharIndex = 'A'; CharIndex <= Char2Glob; ++CharIndex)
                        if (EnumLoc == Func1(CharIndex, 'C'))
                                Proc6(Ident1, &EnumLoc);
                IntLoc3 = IntLoc2 * IntLoc1;
                IntLoc2 = IntLoc3 / IntLoc1;
                IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1;
                Proc2(&IntLoc1);
        }

        /*****************
        -- Stop Timer --
        *****************/

        benchtime = dtime() - startclock;
        
        //printf ("%12.0f runs %6.2f seconds \n",(double) Loops, benchtime);

     } 
   while (0);
 
    Dhrystones_Per_Second = (double) Loops / benchtime;
    Vax_Mips = Dhrystones_Per_Second / 1757.0;
#if 0
    printf ("Dhrystones per Second:                      ");
    printf ("%10.0lf \n", Dhrystones_Per_Second);
    printf ("VAX  MIPS rating =                          ");
    printf ("%12.2lf \n",Vax_Mips);
#endif
}

void Proc1(RecordPtr PtrParIn)
{
#define NextRecord      (*(PtrParIn->PtrComp))

        structassign(NextRecord, *PtrGlb);
        PtrParIn->IntComp = 5;
        NextRecord.IntComp = PtrParIn->IntComp;
        NextRecord.PtrComp = PtrParIn->PtrComp;
        Proc3(&NextRecord.PtrComp);
        if (NextRecord.Discr == Ident1)
        {
                NextRecord.IntComp = 6;
                Proc6(PtrParIn->EnumComp, &NextRecord.EnumComp);
                NextRecord.PtrComp = PtrGlb->PtrComp;
                Proc7(NextRecord.IntComp, 10, &NextRecord.IntComp);
        }
        else
                structassign(*PtrParIn, NextRecord);

#undef  NextRecord
}

void Proc2(OneToFifty *IntParIO)
{
        OneToFifty              IntLoc;
        Enumeration             EnumLoc;

        IntLoc = *IntParIO + 10;
        for(;;)
        {
                if (Char1Glob == 'A')
                {
                        --IntLoc;
                        *IntParIO = IntLoc - IntGlob;
                        EnumLoc = Ident1;
                }
                if (EnumLoc == Ident1)
                        break;
        }
}

void Proc3(RecordPtr *PtrParOut)
{
        if (PtrGlb != NULL)
                *PtrParOut = PtrGlb->PtrComp;
        else
                IntGlob = 100;
        Proc7(10, IntGlob, &PtrGlb->IntComp);
}

void Proc4()
{
        bool BoolLoc;

        BoolLoc = Char1Glob == 'A';
        BoolLoc |= BoolGlob;
        Char2Glob = 'B';
}

void Proc5()
{
        Char1Glob = 'A';
        BoolGlob = FALSE;
}

extern bool Func3();

void Proc6(Enumeration EnumParIn, Enumeration *EnumParOut)
{
        *EnumParOut = EnumParIn;
        if (! Func3(EnumParIn) )
                *EnumParOut = Ident4;
        switch (EnumParIn)
        {
        case Ident1:    *EnumParOut = Ident1; break;
        case Ident2:    if (IntGlob > 100) *EnumParOut = Ident1;
                        else *EnumParOut = Ident4;
                        break;
        case Ident3:    *EnumParOut = Ident2; break;
        case Ident4:    break;
        case Ident5:    *EnumParOut = Ident3;
        }
}

void Proc7(OneToFifty IntParI1, OneToFifty IntParI2, OneToFifty *IntParOut)
{
        OneToFifty      IntLoc;

        IntLoc = IntParI1 + 2;
        *IntParOut = IntParI2 + IntLoc;
}

void Proc8(Array1Dim Array1Par, Array2Dim Array2Par, OneToFifty IntParI1,
                                                OneToFifty IntParI2)
{
        OneToFifty      IntLoc;
        OneToFifty      IntIndex;

        IntLoc = IntParI1 + 5;
        Array1Par[IntLoc] = IntParI2;
        Array1Par[IntLoc+1] = Array1Par[IntLoc];
        Array1Par[IntLoc+30] = IntLoc;
        for (IntIndex = IntLoc; IntIndex <= (IntLoc+1); ++IntIndex)
                Array2Par[IntLoc][IntIndex] = IntLoc;
        ++Array2Par[IntLoc][IntLoc-1];
        Array2Par[IntLoc+20][IntLoc] = Array1Par[IntLoc];
        IntGlob = 5;
}

Enumeration Func1(CapitalLetter CharPar1, CapitalLetter CharPar2)
{
        CapitalLetter   CharLoc1;
        CapitalLetter   CharLoc2;

        CharLoc1 = CharPar1;
        CharLoc2 = CharLoc1;
        if (CharLoc2 != CharPar2)
                return (Ident1);
        else
                return (Ident2);
}

bool Func2(String30 StrParI1, String30 StrParI2)
{
        OneToThirty             IntLoc;
        CapitalLetter           CharLoc;

        IntLoc = 1;
        while (IntLoc <= 1)
                if (Func1(StrParI1[IntLoc], StrParI2[IntLoc+1]) == Ident1)
                {
                        CharLoc = 'A';
                        ++IntLoc;
                }
        if (CharLoc >= 'W' && CharLoc <= 'Z')
                IntLoc = 7;
        if (CharLoc == 'X')
                return(TRUE);
        else
        {
                if (strcmp(StrParI1, StrParI2) > 0)
                {
                        IntLoc += 7;
                        return (TRUE);
                }
                else
                        return (FALSE);
        }
}

bool Func3(Enumeration EnumParIn)
{
        Enumeration     EnumLoc;

        EnumLoc = EnumParIn;
        if (EnumLoc == Ident3) return (TRUE);
        return (FALSE);
}


