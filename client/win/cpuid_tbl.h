//
// cpuid_tbl.h - define structure of ID table
// 
#include "opt_x86.h"


enum	{
	_INTEL=1, _AMD, _CYRIX, _UMC, _NEXGEN, _RISE, _SIS, _TMT, _CENTAUR, _NSC
	};


typedef struct {
	_int8	family,model,xFamily,xModel;
	_int16	brand, cacheL2;
	struct	{
		char *normal;
		char *abrv;
		char *nick;
		} name;
	} CPUID_DEFS;
extern CPUID_DEFS cpuid_tbl[];



typedef struct {
	short	L2Cache, mhz, perfNum;
	}
	AMD_PERFTBL;
extern AMD_PERFTBL perfTbl[];



typedef struct {
		_uint8	idByte, code, data, line;
		_int16	L1, L2, L3;
	} _cacheCodes;
extern _cacheCodes cacheCodes[];
