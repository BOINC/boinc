// This is a place for project specific defines that might be necessary for your
// builds.  Put them here so configure doesn't clobber them
#ifndef _PROJECT_SPECIFIC_DEFINES_H_
#define _PROJECT_SPECIFIC_DEFINES_H_ 1

#ifndef SETIATHOME
#define SETIATHOME 1
// scheduler defines
//    Shared Memory parameters
#define MAX_APP_VERSIONS 100
#define MAX_WU_RESULTS 250
//    Plan Class defines
#define ATI_MIN_RAM 222*MEGA
#define OPENCL_ATI_MIN_RAM ATI_MIN_RAM
#define OPENCL_NVIDIA_MIN_RAM ATI_MIN_RAM
#endif

//#ifndef EINSTEIN_AT_HOME
//#define EINSTEIN_AT_HOME 1
//#endif

//#ifndef _WCG
//#define _WCG 1
//#endif

#endif
