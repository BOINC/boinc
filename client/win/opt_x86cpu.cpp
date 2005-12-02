/***************
 *
 *	opt_x86cpu.hpp:	x86 CPU specific identification routines
 *	Originator:		Ben Herndon
 *
 *   to be used with seti @ home client code
 *
 */

#include <stdio.h>

#include "optimize.hpp"
#include "optBench.hpp"
#include "opt_x86.h"
#include <string.h>

#include "cpuid_tbl.h"
#include "util.h"

#if defined(__GNUC__)

__inline UINT32 CPUID_X86 (UINT32 cmd, UINT32 &eax, UINT32 &ebx, UINT32 &ecx, UINT32 &edx)
{
     asm ("cpuid"
	     : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
	     : "a" (cmd) );
	return eax;
}

#endif // Not __GNUC__

#if defined(_MSC_VER) || defined(_ICC)
static _uint32 CPUID_X86
	(_uint32 command, _uint32 &_eax, _uint32 &_ebx, _uint32 &_ecx, _uint32 &_edx)
{
	__asm {
	    mov eax, [command]
		cpuid			; <<CPUID>>
		push eax
		mov	eax, [_ebx]
		mov	[eax], ebx
		mov	eax, [_ecx]
		mov	[eax], ecx
		mov	eax, [_edx]
		mov	[eax], edx
		mov eax,[_eax]
		pop ebx
		mov	[eax], ebx
	}
	return _eax;
}

#endif

//===========[ PROTOTYPES ]==================

extern _uint32 calcOurMhz(void) ;

//---------------  class  CPU_INFO_t 

#define EXT_CMD 0x80000000

#ifdef __EMX__
#define __forceinline 
#define __fastcall
#endif

static __forceinline bool __fastcall testBit(_uint32 flags, int bitShift) {
	return ((flags & (1<<bitShift)) != 0);
}

//
// CPU_INFO_t - CONSTRUCTOR
//
CPU_INFO_t::CPU_INFO_t	(void) {
	UINT32	dummy, dummy1, flags1, flags2, flags3;

	has_mmx		= 
	has_sse		= 
	has_sse2	= 
	has_sseOS	= 
	has_3Dnow	= 
	has_3DnowPlus = 
	has_mmxPlus	= false;
	cpuMhz 		= 0;
	busMhz 		= 0;

		// 1.	Establish that the processor has support for CPUID. 
	checkCpuidOpcode();
	if( !has_cpuid_opcode)	return;
	
		// 2.	Get vendor string and the highest standard function supported.
	CPUID_X86(0, cpu.max_stdFunc, vendor.id.longs[0], vendor.id.longs[2], vendor.id.longs[1]);
	if(cpu.max_stdFunc < 1)	return;
	
	vendor.id.name[12] = '\0';
	
		// 3. execute CPUID function 1, which returns the standard feature flags in the EDX register.
	CPUID_X86(1, flags1, flags3, dummy, flags2);

	cpu.family	= (flags1 >> 8) & 0x0F;
	cpu.model	= (flags1 >> 4) & 0x0F;
	cpu.rev		= (flags1 >> 0) & 0x0F;
	cpu.type	= (flags1 >>12) & 0x0F;
	cpu.extModel= (flags1 >>16) & 0x0F;
	cpu.extFamily=(flags1 >>20) & 0xFF;

	cpu.brandID	= (flags3 >> 0) & 0xFF;

		// 4. If bit 23 of the standard feature flags is set to 1, MMX technology is supported. 
	has_mmx		= testBit(flags2,23);
	has_sse		= testBit(flags2,25);
	has_sse2	= testBit(flags2,26);
	has_sseOS	= sseOSsupport();

		//6. Execute CPUID extended function 8000_0000h. 
		// This function returns the highest extended function supported in EAX.
	CPUID_X86(EXT_CMD, cpu.max_extFunc, dummy, dummy, dummy);
	cpu.max_extFunc -= (EXT_CMD);


	cpuidLookup();	// Get official company names & CPU names
	cpuidName();
#ifndef __EMX__
	cpuMhz = calcOurMhz();
#endif


		//7. Execute CPUID function 8000_0001h. This function returns the extended feature flags in EDX
	if(cpu.max_extFunc >= 1) {
		CPUID_X86(EXT_CMD + 1, dummy, dummy, dummy, flags2);
		switch(cpu.vendor) {
			case _AMD:
				has_3Dnow	= testBit(flags2,31); //8. If bit 31 of ...is set to 1, 3DNow! supported
				has_3DnowPlus=testBit(flags2,30); //10. If bit 30 of...is set to 1, 3DNow! additions supported
				has_mmxPlus	 =testBit(flags2,22);
				break;
			default:
				break;
			}
	}

	getCacheSizes();
}



//
// getCacheSizes - Determine if CPUID is available
//
void CPU_INFO_t::getCacheSizes(void) {
	UINT32	cacheBits[4] = { 0, 0, 0, 0 };
	int		theSize, codeN, dataN, cache1N, cache2N, cache3N, line;

	cache.code = codeN = 0;
	cache.data = dataN = 0;
	cache.L1 = cache1N = 0;
	cache.L2 = cache2N = 0;
	cache.L3 = cache3N = 0;
	
	if (cpu.max_extFunc >= 5) {
		CPUID_X86(EXT_CMD + 5, cacheBits[0], cacheBits[1], cacheBits[2], cacheBits[3]);
		
		theSize  = (cacheBits[2] >>24) & 0xFF;
		theSize += (cacheBits[3] >>24) & 0xFF;
		cache.L1 = theSize;
		cache.line_size = cacheBits[3] & 0xFF;
	}
	
	if (cpu.max_extFunc >= 6) {
		CPUID_X86(EXT_CMD + 6, cacheBits[0], cacheBits[1], cacheBits[2], cacheBits[3]);

		cache.L2 = (cacheBits[3] >>16) & 0xFFFF;
	}

	if ( cache.L1 + cache.L2 == 0)
	  if ( cpu.vendor == _INTEL && cpu.max_stdFunc >= 2) {
		CPUID_X86(2, cacheBits[0], cacheBits[1], cacheBits[2], cacheBits[3]);
		
		_uint8 *bytePtr = (_uint8 *) &cacheBits;
		for(int x = 0 ; x < sizeof(cacheBits); x ++, bytePtr++) {
			if( x % 4 == 0) continue;
			 for(int y = 0; cacheCodes[y].idByte != 0xFF; y++) {
			 	if(*bytePtr != cacheCodes[y].idByte)
			 		continue;
				cache.L1 += cacheCodes[y].L1;
				cache.L2 += cacheCodes[y].L2;
				cache.L3 += cacheCodes[y].L3;
				cache.code += cacheCodes[y].code;
				cache.data += cacheCodes[y].data;
				if( cacheCodes[y].line > cache.line_size)
					cache.line_size = cacheCodes[y].line;
			}
		}

	  if( cache.L1 + cache.L2 == 0)	// Still no official cache?
		cache.L1 = cache.code + cache.data;
	  }
}


//
// checkCpuidOpcode - Determine if CPUID is available
//
void CPU_INFO_t::checkCpuidOpcode(void) {
	UINT32	CPUID_valid;

	has_cpuid_opcode = true;
	return;

#if defined(__GNUC__)

		asm ("\n\t sti \n\t pushfd \n\t pop %%eax \n\t mov %%edx,%%eax \n\t xor %%eax, 0x0200000" \
		"\n\t push %%eax \n\t popfd \n\t pushfd \n\t pop %%eax \n\t cli \n\t xor %%eax, %%edx"
			:
			: "a" (CPUID_valid) );
#else
		_asm {
			sti
			pushfd                      ; save EFLAGS to stack.
			pop     eax                 ; store EFLAGS in eax.
			mov     edx, eax            ; save in edx for testing later.
			xor     eax, 0200000h       ; switch bit 21.
			push    eax                 ; copy "changed" value to stack.
			popfd                       ; save "changed" eax to EFLAGS.
			pushfd
			cli
			pop     eax
			xor     eax, edx            ; See if bit changeable.
			mov     CPUID_valid, eax	; Save the value in eax to a variable.
		}
#endif

	has_cpuid_opcode = ( CPUID_valid != 0);
}

#if defined(__GNUC__)
#define fxsave(addr)  asm("fxsave %0" : "=m" (addr))
#define fxrstor(addr) asm("fxrstor %0" : "=m" (addr))
#define testOrps() asm("orps xmm1, xmm1");
#endif

#if defined(_MSC_VER)
#define fxsave(addr)  __asm{ fxsave addr }
#define fxrstor(addr)  __asm{ fxrstor addr }
#define testOrps() __asm{ orps	xmm1, xmm1 }
#endif

//======================
//
// sseOSsupport - Check for OS support of SSE instruction
//
bool CPU_INFO_t::sseOSsupport(void) {
	if(!has_sse)	return false;

#ifdef __EMX__
	return false;
#else
	try	{ testOrps(); }
	catch(int err) {	return false; }
	return true;
#endif
}

/*
	char	fxSaveBuf[512+32], *fxSavePtr;
	int		*xmm6Reg = (int *) &fxSaveBuf[0x10C];
	int		testVal = 'BenH', save1, save2, save3;
	
	fxSavePtr = (char*)((int)(fxSaveBuf+15) & ~15);

	fxsave(fxSavePtr);	// Save it
	save1	= *xmm6Reg;	// Save original value
	*xmm6Reg ^= testVal;
	fxrstor(fxSavePtr);	// Update xmm6 (if possible)
	save2 = *xmm6Reg;
	fxsave(fxSavePtr);	// Save it
	save3 = *xmm6Reg;
	*xmm6Reg = save1;
	fxrstor(fxSavePtr);	// Put back original value
	return ( save2 ^ save3 == testVal);
*/



//===============================
//
// cpuidLookup - Find system details with info from cpuid
//
void CPU_INFO_t::cpuidLookup(void) {
	CPUID_DEFS	*idDef;
	bool	found = false;
	
	for( idDef = cpuid_tbl; !found ; idDef++) {

			// Find a vendor entry [0,0,0,0]
		while(idDef->family + idDef->model + idDef->xFamily + idDef->xModel)
			 idDef++;

			// End of list marker?
		if( idDef->cacheL2 == -1)
			break;
			
			// Vendor code name match?
			// Table Name format:  "[" + <vendorStr> + "]"
		if( strncmp (vendor.id.name, &idDef->name.normal[1], 12) != 0)
			continue;

		cpu.vendor = idDef->cacheL2;
		vendor.abrvName = idDef->name.abrv;
		vendor.company	= idDef->name.nick;
		
			// Special case... _AMD & _TMT have 2 vendor.id.name types
			//  so if name matched first, skip the 2nd 
		if(idDef->cacheL2 == idDef[1].cacheL2)
			 idDef++;
			
			// Now find Family/Model match
		for(idDef++  ; !found && idDef->family; idDef++) {
			if(idDef->family < cpu.family || idDef->model < cpu.model)
				continue;
			if(idDef->xFamily > 0 && idDef->xFamily < cpu.extFamily)
				continue;
			if(idDef->xModel > 0  && idDef->xModel < cpu.extModel)	// extModel used?
				continue;
			if(idDef->brand > 0   && idDef->brand < cpu.brandID)	// Brand matters?
				continue;
			if(idDef->cacheL2 > 0 && idDef->cacheL2 < cache.L2)		// Cache matters?
				continue;

			found = true;
			name.normal = idDef->name.normal;
			name.abrv = idDef->name.abrv;
			name.nick = idDef->name.nick;
			}
	}
}


//===============================
//
// cpuidName - Get supplied name from cpuid
//
void CPU_INFO_t::cpuidName(void) {
	
	name.fromID[0] = '\0';	// Terminate name
	
	if (cpu.max_extFunc <4)
		return;

		// Prepare to build the name from a series of 32 bit words
	_uint32	*namePtr = (_uint32	*)&name.fromID;
	
	CPUID_X86(EXT_CMD + 2, namePtr[0], namePtr[1], namePtr[2], namePtr[3]);
	CPUID_X86(EXT_CMD + 3, namePtr[4], namePtr[5], namePtr[6], namePtr[7]);
	CPUID_X86(EXT_CMD + 4, namePtr[8], namePtr[9], namePtr[10], namePtr[11]);
	name.fromID[12*4] = '\0';	// Terminate name
	strip_whitespace(name.fromID);	// trim whitespace [>_INTEL<]
}

#include <time.h>
#ifndef __EMX__
#include <winbase.h>
#endif

const unsigned int million = 1000000;
double calcFreq(int num_tics);

_uint32 calcOurMhz(void) {
	double avgFreq, currFreq, tmp, avgDev, wantedDev;
	_int32	loops;

	wantedDev = 1.0;
	avgFreq = calcFreq(50);
	loops = 0;
	do	{
		currFreq=calcFreq(50);
		avgFreq =(currFreq+avgFreq)/2;

		// Work out the current average deviation of the frequency.
		tmp += fabs(currFreq-avgFreq);
		avgDev = tmp/++loops;
	}	while( loops < 25 && avgDev > wantedDev);

	return avgFreq/million;
}

//
//
//
double calcFreq(int num_tics) {
	clock_t start, begin, end, goal;
	_int64	timer1, timer2;

	start	= clock();
	goal	= start + num_tics + 1;
	while( clock() == start)	// wait for leading edge change
		;
	timer1	= cycleCount();
	do	{
		end	= clock();
	} while( end < goal);
	timer2 = cycleCount();
	return double(CLOCKS_PER_SEC)/(end-start-1)*4/3
		* (timer2-timer1);
}

const char *BOINC_RCSID_4f1d3f9017 = "$Id$";
