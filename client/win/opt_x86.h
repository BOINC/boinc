/***************
 *
 *	opt_x86.h:	x86 CPU definitions, inline functions and macros
 *	Originator:		Ben Herndon
 *
 *   to be used with seti @ home client code
 *
 */

#ifndef OPT_X86_H
#define OPT_X86_H


//---------- Constants ---------//

#if defined(__GNUC__)
typedef unsigned long long int _uint64;
typedef unsigned int	_uint32;
typedef unsigned short	_uint16;
typedef unsigned char 	_uint8;

typedef long long int	_int64;
typedef int				_int32;
typedef short 			_int16;
typedef char  			_int8;

typedef unsigned long long int _uint64;
typedef unsigned int	_uint32;
typedef unsigned short	UINT16;
typedef unsigned char 	UINT8;

typedef long long int	INT64;
typedef int				INT32;
typedef short 			INT16;
typedef char  			INT8;
#endif


#if defined(_MSC_VER)
typedef unsigned long long int _uint64;
typedef unsigned int	_uint32;
typedef unsigned short	_uint16;
typedef unsigned char 	_uint8;

#endif

/*	__SSE__		
	UNPCKHPS	(CAca) = (AaBb, CcDd)
	UNPCKLPS	(Dbdb) = (AaBb, CcDd)

	MOVHPS		(MmBb) = (AaBb, Mm)
	MOVLPS		(AaMm) = (AaBb, Mm)
	MOVHLPS		(AaDd) = (AaBb, CcDd)
	MOVLHPS		(CcBb) = (AaBb, CcDd)

	__3DNow__
	+PFNACC		([b-B][a-A])	= (Aa, Bb)
	+PFPNACC 	([b+B][a-A])	= (Aa, Bb)
	+PSWAPD	 	(bB)			= (Aa, Bb)
	
	PFACC 	 	([A+a][B+b])	= (Aa, Bb)
	PFADD 	 	([A+B][a+b])	= (Aa, Bb)
	PFMUL	 	([A*B][a*b])	= (Aa, Bb)
	PFSUB	 	([A-B][a-b])	= (Aa, Bb)
	PFSUBR	 	([B-A][B-A])	= (Aa, Bb)
	PFMAX	 	([max(A,B)][max(a,b)])	= (Aa, Bb)
	PFSUB	 	([A-B][a-b])	= (Aa, Bb)
	OPCODE	 	(xx)	= (Aa, Bb)
*/

//---------- SSE macros ---------//

//  s_get(p)  - GET 4 floats from unaligned address
//  s_put(p)	- PUT 4 floats at unaligned address
//  FILLsim(a,b,c,d) - GET 4 floats directly into register
//  s_add(a,b)	- Add 4 floats to another 4 floats, return result - Alignment or registers required
//  s_div(a,b)	- Divide 4 floats in a by those in b
//  s_max(a,b)	- Return the 4 highest floats between a and b

// && (defined(__i386__) || defined(__x86_64__))


#if defined(__3DNOW__) && defined(__GNUC__)	// GCC's versions of sse

#endif //defined(__3DNOW__)

//---------- Prototypes ---------//

//*****
//
// cycleCount - Machine specific function to get cycle counter
//
_uint64 cycleCount( void);

//
// x86_fsincos - Calculate sin & cos at same time
//
void x86_fsincos (double angle, double &sinVal, double &cosVal);

#if defined(__GNUC__)
__inline _uint64 cycleCount( void) {
	_uint64 cycles;
	_uint32	dummy;
		// Use CPUID to stop the instruction pipeline and get starting cycle
	asm volatile("cpuid" : "=A" (cycles), "=b" (dummy), "=c" (dummy));	
	asm volatile("rdtsc" : "=A" (cycles) );
	return (cycles);
}

// Only available on i387 FPU and above
#if (defined(__i386__) || defined(__x86_64__))
__inline void x86_fsincos (double angle, double *sinVal, double *cosVal) {
	asm ("fsincos" : "=t" (cosVal), "=u" (sinVal) : "0" (angle));
}
__inline void x86_fsincos (double angle, float *sinVal, float *cosVal) {
	asm ("fsincos" : "=t" (cosVal), "=u" (sinVal) : "0" (angle));
}
#endif
#endif	// __GNUC


#if (defined(_MSC_VER) && defined(_M_IX86))
__inline _uint64 cycleCount( void) {
	_uint64 cycles;
		// Use CPUID to stop the instruction pipeline and get starting cycle
	__asm {
		push ebx
		push ecx
		xor	eax,eax
		cpuid
		rdtsc
		mov	DWORD PTR [cycles+0],eax
		mov	DWORD PTR [cycles+4],edx
		pop	ecx
		pop	ebx
	}	
	return (cycles);
}

#define HAVE_SINCOS 1
__inline void x86_fsincos (double angle, double *sinVal, double *cosVal) {
	__asm	{
		fld	[angle]
		fsincos
		mov	eax,cosVal
		fstp	qword ptr [eax]
		mov	eax,sinVal
		fstp	qword ptr [eax]
	}
}
__inline void x86_fsincos (double angle, float *sinVal, float *cosVal) {
	__asm	{
		fld	[angle]
		fsincos
		mov	eax,cosVal
		fstp	dword ptr [eax]
		mov	eax,sinVal
		fstp	dword ptr [eax]
	}
}
#endif

//---------- Classes ---------//


//
// CLASS 
// 

typedef struct {
	int		L1, L2, L3;
	int		code, data;
	int		line_size;
	} CACHE_INFO;

class CPU_INFO_t {
	public:
		CPU_INFO_t();
		~CPU_INFO_t() {};

		int		cacheL1() { return cache.L1 * 1024; };
		int		cacheL2() { return cache.L2 * 1024; };
		int		cacheLine() { return cache.line_size; };
		CACHE_INFO & getCache(void) { return cache; };

		bool	has_cpuid_opcode;
		bool	has_mmx;
		bool	has_mmxPlus;
		bool	has_3Dnow;
		bool	has_3DnowPlus;
		bool	has_sse;
		bool	has_sseOS;
		bool	has_sse2;
	
		struct	{
			int		vendor;
			_uint32	max_stdFunc;
			_uint32	max_extFunc;
			int		family;	//	3 - 386 family, 4 - i486 family, 5 - Pentium family, 6 - Pentium Pro family
			int		model;	//
			int		rev;
			int		type;
			int		extFamily;
			int		extModel;
			int		brandID;
			} cpu; 
		CACHE_INFO	cache;
		_uint32	cpuMhz;
		_uint32	busMhz;

		struct	{
			union	{
				_uint32	longs[3];
				char	name[16];
				} id;
				char	*abrvName;
				char	*company;
			} vendor;

		struct	{
			char  fromID[64];
			char *normal;
			char *abrv;
			char *nick;
			} name;

	private:
		void	checkCpuidOpcode(void);	// Use generic code to detect w/o causing bad opcode
		void	getCacheSizes(void);
		bool	sseOSsupport(void);
		void	cpuidLookup(void);
		void	cpuidName(void);
	};				
			

#endif  // #ifndef OPT_X86_H
