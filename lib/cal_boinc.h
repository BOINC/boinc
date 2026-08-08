/**
 *  @file     cal.h
 *  @brief    CAL Interface Header
 *  @version  1.00.0 Beta
 */


/* ============================================================

Copyright (c) 2007 Advanced Micro Devices, Inc.  All rights reserved.

Redistribution and use of this material is permitted under the following
conditions:

Redistributions must retain the above copyright notice and all terms of this
license.

In no event shall anyone redistributing or accessing or using this material
commence or participate in any arbitration or legal action relating to this
material against Advanced Micro Devices, Inc. or any copyright holders or
contributors. The foregoing shall survive any expiration or termination of
this license or any agreement or access or use related to this material.

ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE REVOCATION
OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERATION, OR THAT IT IS FREE
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT.
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES,
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES,
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S.
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS,
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS,
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS.
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE REGARDING THE U.S. GOVERNMENT AND DOD AGENCIES: This material is
provided with "RESTRICTED RIGHTS" and/or "LIMITED RIGHTS" as applicable to
computer software and technical data, respectively. Use, duplication,
distribution or disclosure by the U.S. Government and/or DOD agencies is
subject to the full extent of restrictions in all applicable regulations,
including those found at FAR52.227 and DFARS252.227 et seq. and any successor
regulations thereof. Use of this material by the U.S. Government and/or DOD
agencies is acknowledgment of the proprietary rights of any copyright holders
and contributors, including those of Advanced Micro Devices, Inc., as well as
the provisions of FAR52.227-14 through 23 regarding privately developed and/or
commercial computer software.

This license forms the entire agreement regarding the subject matter hereof and
supersedes all proposals and prior discussions and writings between the parties
with respect thereto. This license does not affect any ownership, rights, title,
or interest in, or relating to, this material. No terms of this license can be
modified or waived, and no breach of this license can be excused, unless done
so in a writing signed by all affected parties. Each term of this license is
separately enforceable. If any term of this license is determined to be or
becomes unenforceable or illegal, such term shall be reformed to the minimum
extent necessary in order for this license to remain in effect in accordance
with its terms as modified by such reformation. This license shall be governed
by and construed in accordance with the laws of the State of Texas without
regard to rules on conflicts of law of any state or jurisdiction or the United
Nations Convention on the International Sale of Goods. All disputes arising out
of this license shall be subject to the jurisdiction of the federal and state
courts in Austin, Texas, and all defenses are hereby waived concerning personal
jurisdiction and venue of these courts.

============================================================ */



#ifndef __CAL_H__
#define __CAL_H__

#ifdef __cplusplus
extern "C" {
#define CALAPI
#else
#define CALAPI extern
#endif

#ifdef _WIN32
#define CALAPIENTRY  __stdcall
#else
#define CALAPIENTRY
#endif

typedef void           CALvoid;       /**< void type                        */
typedef char           CALchar;       /**< ASCII character                  */
typedef signed   char  CALbyte;       /**< 1 byte signed integer value      */
typedef unsigned char  CALubyte;      /**< 1 byte unsigned integer value    */
typedef signed   short CALshort;      /**< 2 byte signed integer value      */
typedef unsigned short CALushort;     /**< 2 byte unsigned integer value    */
typedef signed   int   CALint;        /**< 4 byte signed integer value      */
typedef unsigned int   CALuint;       /**< 4 byte unsigned intger value     */
typedef float          CALfloat;      /**< 32-bit IEEE floating point value */
typedef double         CALdouble;     /**< 64-bit IEEE floating point value */
typedef signed   long  CALlong;       /**< long value                       */
typedef unsigned long  CALulong;      /**< unsigned long value              */

#if defined(_MSC_VER)

typedef signed __int64     CALint64;  /**< 8 byte signed integer value */
typedef unsigned __int64   CALuint64; /**< 8 byte unsigned integer value */

#elif defined(__GNUC__)

typedef signed long long   CALint64;  /**< 8 byte signed integer value */
typedef unsigned long long CALuint64; /**< 8 byte unsigned integer value */

#else
#error "Unsupported compiler type."
#endif



/** Boolean type */
typedef enum CALbooleanEnum {
    CAL_FALSE          = 0,           /**< Boolean false value */
    CAL_TRUE           = 1            /**< Boolean true value */
} CALboolean;

/** Function call result/return codes */
typedef enum CALresultEnum {
    CAL_RESULT_OK                = 0, /**< No error */
    CAL_RESULT_ERROR             = 1, /**< Operational error */
    CAL_RESULT_INVALID_PARAMETER = 2, /**< Parameter passed in is invalid */
    CAL_RESULT_NOT_SUPPORTED     = 3, /**< Function used properly but currently not supported */
    CAL_RESULT_ALREADY           = 4, /**< Stateful operation requested has already been performed */
    CAL_RESULT_NOT_INITIALIZED   = 5, /**< CAL function was called without CAL being initialized */
    CAL_RESULT_BAD_HANDLE        = 6, /**< A handle parameter is invalid */
    CAL_RESULT_BAD_NAME_TYPE     = 7, /**< A name parameter is invalid */
    CAL_RESULT_PENDING           = 8, /**< An asynchronous operation is still pending */
    CAL_RESULT_BUSY              = 9,  /**< The resource in question is still in use */
    CAL_RESULT_WARNING           = 10, /**< Compiler generated a warning */
} CALresult;

/** Data format representation */
typedef enum CALformatEnum {
    CAL_FORMAT_UBYTE_1,     /**< A 1 component 8-bit  unsigned byte format    */
    CAL_FORMAT_UBYTE_2,     /**< A 2 component 8-bit  unsigned byte format    */
    CAL_FORMAT_UBYTE_4,     /**< A 4 component 8-bit  unsigned byte format    */
    CAL_FORMAT_USHORT_1,    /**< A 1 component 16-bit unsigned short format   */
    CAL_FORMAT_USHORT_2,    /**< A 2 component 16-bit unsigned short format   */
    CAL_FORMAT_USHORT_4,    /**< A 4 component 16-bit unsigned short format   */
    CAL_FORMAT_UINT_4,      /**< A 4 component 32-bit unsigned integer format */
    CAL_FORMAT_BYTE_4,      /**< A 4 component 8-bit  byte format    */
    CAL_FORMAT_SHORT_1,     /**< A 1 component 16-bit short format   */
    CAL_FORMAT_SHORT_2,     /**< A 2 component 16-bit short format   */
    CAL_FORMAT_SHORT_4,     /**< A 4 component 16-bit short format   */
    CAL_FORMAT_FLOAT_1,     /**< A 1 component 32-bit float format   */
    CAL_FORMAT_FLOAT_2,     /**< A 2 component 32-bit float format   */
    CAL_FORMAT_FLOAT_4,     /**< A 4 component 32-bit float format   */
    CAL_FORMAT_DOUBLE_1,    /**< A 1 component 64-bit float format   */
    CAL_FORMAT_DOUBLE_2,    /**< A 2 component 64-bit float format   */
    CAL_FORMAT_UINT_1,      /**< A 1 component 32-bit unsigned integer format */
    CAL_FORMAT_UINT_2,      /**< A 2 component 32-bit unsigned integer format */
    CAL_FORMAT_BYTE_1,      /**< A 1 component 8-bit  byte format    */
    CAL_FORMAT_BYTE_2,      /**< A 2 component 8-bit  byte format    */
    CAL_FORMAT_INT_1,       /**< A 1 component 32-bit signed integer format */
    CAL_FORMAT_INT_2,       /**< A 2 component 32-bit signed integer format */
    CAL_FORMAT_INT_4,       /**< A 4 component 32-bit signed integer format */
} CALformat;

/** Device Kernel ISA */
typedef enum CALtargetEnum {
    CAL_TARGET_600,                /**< R600 GPU ISA */
    CAL_TARGET_610,                /**< RV610 GPU ISA */
    CAL_TARGET_630,                /**< RV630 GPU ISA */
    CAL_TARGET_670,                /**< RV670 GPU ISA */
    CAL_TARGET_7XX,                /**< R700 class GPU ISA */
    CAL_TARGET_770,                /**< RV770 GPU ISA */
    CAL_TARGET_710,                /**< RV710 GPU ISA */
    CAL_TARGET_730,                /**< RV730 GPU ISA */
} CALtarget;

/** CAL object container */
typedef struct CALobjectRec* CALobject;

/** CAL image container */
typedef struct CALimageRec*  CALimage;

typedef CALuint CALdevice;      /**< Device handle */
typedef CALuint CALcontext;     /**< context */
typedef CALuint CALresource;    /**< resource handle */
typedef CALuint CALmem;         /**< memory handle */
typedef CALuint CALfunc;        /**< function handle */
typedef CALuint CALname;        /**< name handle */
typedef CALuint CALmodule;      /**< module handle */
typedef CALuint CALevent;       /**< event handle */

/** CAL computational domain */
typedef struct CALdomainRec {
    CALuint x;                 /**< x origin of domain */
    CALuint y;                 /**< y origin of domain */
    CALuint width;             /**< width of domain */
    CALuint height;            /**< height of domain */
} CALdomain;

/** CAL device information */
typedef struct CALdeviceinfoRec {
    CALtarget  target;              /**< Device Kernel ISA  */
    CALuint    maxResource1DWidth;  /**< Maximum resource 1D width */
    CALuint    maxResource2DWidth;  /**< Maximum resource 2D width */
    CALuint    maxResource2DHeight; /**< Maximum resource 2D height */
} CALdeviceinfo;

/** CAL device attributes */
typedef struct CALdeviceattribsRec {
    CALuint    struct_size;         /**< Client filled out size of CALdeviceattribs struct */
    CALtarget  target;              /**< Asic identifier */
    CALuint    localRAM;            /**< Amount of local GPU RAM in megabytes */
    CALuint    uncachedRemoteRAM;   /**< Amount of uncached remote GPU memory in megabytes */
    CALuint    cachedRemoteRAM;     /**< Amount of cached remote GPU memory in megabytes */
    CALuint    engineClock;         /**< GPU device clock rate in megahertz */
    CALuint    memoryClock;         /**< GPU memory clock rate in megahertz */
    CALuint    wavefrontSize;       /**< Wavefront size */
    CALuint    numberOfSIMD;        /**< Number of SIMDs */
    CALboolean doublePrecision;     /**< double precision supported */
    CALboolean localDataShare;      /**< local data share supported */
    CALboolean globalDataShare;     /**< global data share supported */
    CALboolean globalGPR;           /**< global GPR supported */
    CALboolean computeShader;       /**< compute shader supported */
    CALboolean memExport;           /**< memexport supported */
    CALuint    pitch_alignment;     /**< Required alignment for calCreateRes allocations (in data elements) */
    CALuint    surface_alignment;   /**< Required start address alignment for calCreateRes allocations (in bytes) */
} CALdeviceattribs;

/** CAL device status */
typedef struct CALdevicestatusRec {
    CALuint   struct_size;            /**< Client filled out size of CALdevicestatus struct */
    CALuint   availLocalRAM;          /**< Amount of available local GPU RAM in megabytes */
    CALuint   availUncachedRemoteRAM; /**< Amount of available uncached remote GPU memory in megabytes */
    CALuint   availCachedRemoteRAM;   /**< Amount of available cached remote GPU memory in megabytes */
} CALdevicestatus;

/** CAL resource allocation flags **/
typedef enum CALresallocflagsEnum {
    CAL_RESALLOC_GLOBAL_BUFFER  = 1, /**< used for global import/export buffer */
    CAL_RESALLOC_CACHEABLE      = 2, /**< cacheable memory? */
} CALresallocflags;

/** CAL computational 3D domain */
typedef struct CALdomain3DRec {
    CALuint width;             /**< width of domain */
    CALuint height;            /**< height of domain */
    CALuint depth;             /**< depth  of domain */
} CALdomain3D;

/** CAL computational grid */
typedef struct CALprogramGridRec {
    CALfunc     func;          /**< CALfunc to execute */
    CALdomain3D gridBlock;     /**< size of a block of data */
    CALdomain3D gridSize;      /**< size of 'blocks' to execute. */
    CALuint     flags;         /**< misc grid flags */
} CALprogramGrid;

/** CAL computational grid array*/
typedef struct CALprogramGridArrayRec {
    CALprogramGrid* gridArray;/**< array of programGrid structures */
    CALuint     num;           /**< number of entries in the grid array */
    CALuint     flags;         /**< misc grid array flags */
} CALprogramGridArray;

/** CAL function information **/
typedef struct CALfuncInfoRec
{
    CALuint    maxScratchRegsNeeded;    /**< Maximum number of scratch regs needed */
    CALuint    numSharedGPRUser;        /**< Number of shared GPRs */
    CALuint    numSharedGPRTotal;       /**< Number of shared GPRs including ones used by SC */
    CALboolean eCsSetupMode;            /**< Slow mode */
    CALuint    numThreadPerGroup;       /**< Number of threads per group */
    CALuint    totalNumThreadGroup;     /**< Total number of thread groups */
    CALuint    wavefrontPerSIMD;        /**< Number of wavefronts per SIMD */ //CAL_USE_SC_PRM
    CALuint    numWavefrontPerSIMD;     /**< Number of wavefronts per SIMD */
    CALboolean isMaxNumWavePerSIMD;     /**< Is this the max num active wavefronts per SIMD */
    CALboolean setBufferForNumGroup;    /**< Need to set up buffer for info on number of thread groups? */
} CALfuncInfo;

/*============================================================================
 * CAL Runtime Interface
 *============================================================================*/

/*----------------------------------------------------------------------------
 * CAL Subsystem Functions
 *----------------------------------------------------------------------------*/

/**
 * @fn calInit(void)
 *
 * @brief Initialize the CAL subsystem.
 *
 * Initializes the CAL system for computation. The behavior of CAL methods is
 * undefined if the system is not initialized.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error, and CAL_RESULT_ALREADY
 * of CAL has already been initialized.
 *
 * @sa calShutdown
 */
CALAPI CALresult CALAPIENTRY calInit(void);


/**
 * @fn calGetVersion(CALuint* major, CALuint* minor, CALuint* imp)
 *
 * @brief Retrieve the CAL version that is loaded
 *
 * CAL version is in the form of API_Major.API_Minor.Implementation where
 * "API_Major" is the major version number of the CAL API. "API_Minor" is the
 * minor version number of the CAL API. "Implementation" is the implementation
 * instance of the supplied API version number.
 *
 * @return Returns CAL_RESULT_OK on success.
 *
 * @sa calInit calShutdown
 */
CALAPI CALresult CALAPIENTRY calGetVersion(CALuint* major, CALuint* minor, CALuint* imp);

/**
 * @fn calShutdown(void)
 *
 * @brief Shuts down the CAL subsystem.
 *
 * Shuts down the CAL system. calShutdown should always be paired with
 * calInit. An application may have any number of calInit - calShutdown
 * pairs. Any CAL call outsied calInit - calShutdown pair will return
 * CAL_RESULT_NOT_INITIALIZED.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calInit
 */
CALAPI CALresult CALAPIENTRY calShutdown(void);


/*----------------------------------------------------------------------------
 * CAL Device Functions
 *----------------------------------------------------------------------------*/

/**
 * @fn calDeviceGetCount(CALuint* count)
 *
 * @brief Retrieve the number of devices available to the CAL subsystem.
 *
 * Returns in *count the total number of supported GPUs present in the system.
 *
 * @param count (out) - the number of devices available to CAL. On error, count will be zero.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetInfo calDeviceOpen calDeviceClose
 */
CALAPI CALresult CALAPIENTRY calDeviceGetCount(CALuint* count);

/**
 * @fn calDeviceGetInfo(CALdeviceinfo* info, CALuint ordinal)
 *
 * @brief Retrieve information about a specific device available to the CAL subsystem.
 *
 * Returns the device specific information in *info. calDeviceGetInfo returns
 * CAL_RESULT_ERROR if the ordinal is not less than the *count returned in
 * calDeviceGetCount. The target instruction set, the maximum width of
 * 1D resources, the maximum width and height of 2D resources are part
 * of the CALdeviceinfo structure.
 *
 * @param info (out) - the device descriptor struct for the specified device.
 * @param ordinal (in) - zero based index of the device to retrieve information.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetCount calDeviceOpen calDeviceClose
 */
CALAPI CALresult CALAPIENTRY calDeviceGetInfo(CALdeviceinfo* info, CALuint ordinal);

/**
 * @fn calDeviceGetAttribs(CALdeviceattribs* attribs, CALuint ordinal)
 *
 * @brief Retrieve information about a specific device available to the CAL subsystem.
 *
 * Returns the device specific attributes in *attribs. calDeviceGetAttribs returns
 * CAL_RESULT_ERROR if the ordinal is not less than the *count returned in
 * calDeviceGetCount.
 *
 * @param attribs (out) - the device attribute struct for the specified device.
 * @param ordinal (in) - zero based index of the device to retrieve information.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetCount calDeviceOpen calDeviceClose
 */
CALAPI CALresult CALAPIENTRY calDeviceGetAttribs(CALdeviceattribs* attribs, CALuint ordinal);


/**
 * @fn calDeviceGetStatus(CALdevicestatus* status, CALdevice device)
 *
 * @brief Retrieve information about a specific device available to the CAL subsystem.
 *
 * Returns the current status of an open device in *status.
 *
 * @param status (out) - the status struct for the specified device.
 * @param device (in) - handle of the device from which status is to be retrieved.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetAttribs calDeviceOpen calDeviceClose
 */
CALAPI CALresult CALAPIENTRY calDeviceGetStatus(CALdevicestatus* status, CALdevice device);

/**
 * @fn calDeviceOpen(CALdevice* dev, CALuint ordinal)
 *
 * @brief Open the specified device.
 *
 * Opens a device. A device has to be closed before it can be opened again in
 * the same application. This call should always be paired with calDeviceClose.
 * Open the device indexed by the <i>ordinal</i> parameter, which
 * is an unsigned integer in the range of zero to the number of available devices (minus one).
 *
 * @param dev (out) - the device handle for the specified device. On error, dev will be zero.
 * @param ordinal (in) - zero based index of the device to retrieve information.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetCount calDeviceGetInfo calDeviceClose
 */
CALAPI CALresult CALAPIENTRY calDeviceOpen(CALdevice* dev, CALuint ordinal);

/**
 * @fn calDeviceClose(CALdevice dev)
 *
 * @brief Close the specified device.
 *
 * Close the device specified by <i>dev</i> parameter. The
 *
 * @param dev (in) - the device handle for the device to close
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetCount calDeviceGetInfo calDeviceOpen
 */
CALAPI CALresult CALAPIENTRY calDeviceClose(CALdevice dev);


/*----------------------------------------------------------------------------
 * CAL Resource Functions
 *----------------------------------------------------------------------------*/

/**
 * @fn calResAllocLocal2D(CALresource* res, CALdevice dev, CALuint width, CALuint height, CALformat format, CALuint flags)
 *
 * @brief Allocate a memory resource local to a device
 *
 * allocates memory resource local to a device <i>dev</i> and returns a
 * resource handle in <i>*res</i> if successful. This memory is structured
 * as a 2 dimensional region of <i>width</i> and <i>height</i> with a <i>format</i>.
 * The maximum values of <i>width</i> and <i>height</i> are available through
 * the calDeviceGetInfo function. The call returns CAL_RESULT_ERROR if requested
 * memory was not available.
 *
 * Initial implementation will allow this memory to be accessible by all contexts
 * created on this device only. Contexts residing on other devices cannot access
 * this memory.
 *
 * <i>flags</i> can be zero or CAL_RESALLOC_GLOBAL_BUFFER
 * - to specify that the resource will be used as a global
 *   buffer.
 *
 * There are some performance implications when <i>width</i> is not a multiple
 * of 64 for R6xx GPUs.
 *
 * @param res (out)   - returned resource handle. On error, res will be zero.
 * @param dev (in)    - device the resource should be local.
 * @param width (in)  - width of resource (in elements).
 * @param height (in) - height of the resource (in elements).
 * @param format (in) - format/type of each element of the resource.
 * @param flags (in) - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
CALAPI CALresult CALAPIENTRY calResAllocLocal2D(CALresource* res, CALdevice dev, CALuint width, CALuint height, CALformat format, CALuint flags);

/**
 * @fn calResAllocRemote2D(CALresource* res, CALdevice* dev, CALuint devCount, CALuint width, CALuint height, CALformat format, CALuint flags)
 *
 * @brief Allocate a memory resource remote to a set of devices
 *
 * allocates memory resource global to <i>devCount</i> number of devices in <i>dev</i> array
 * and returns a resource handle in <i>*res</i> if successful. This memory is structured
 * as a 2 dimensional region of <i>width</i> and <i>height</i> with a <i>format</i>.
 * The maximum values of <i>width</i> and <i>height</i> are available through
 * the calDeviceGetInfo function. The call returns CAL_RESULT_ERROR if requested
 * memory was not available.
 *
 * Currently only a single device is functional (<i>devCount</i> must be 1).
 *
 * Initial implementation will allow this memory to be accessible by all contexts
 * created on this device only. Contexts residing on other devices cannot access
 * this memory.
 *
 * <i>flags</i> can be zero or CAL_RESALLOC_GLOBAL_BUFFER - to
 * specify that the resource will be used as a global buffer or
 * CAL_RESALLOC_CACHEABLE for GART cacheable memory.
 *
 * One of the benefits with devices being able to write to remote (i.e. system)
 * memory is performance. For example, with large computational kernels, it is
 * sometimes faster for the GPU contexts to write directly to remote
 * memory than it is to do these in 2 steps of GPU context writing to local memory
 * and copying data from GPU local memory to remote system memory via calMemCopy
 *
 * @param res (out)   - returned resource handle. On error, res will be zero.
 * @param dev (in)    - list of devices the resource should be available to.
 * @param devCount (in) - number of devices in the device list.
 * @param width (in)  - width of resource (in elements).
 * @param height (in) - height of the resource (in elements).
 * @param format (in) - format/type of each element of the resource.
 * @param flags (in) - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
CALAPI CALresult CALAPIENTRY calResAllocRemote2D(CALresource* res, CALdevice *dev, CALuint deviceCount, CALuint width, CALuint height, CALformat format, CALuint flags);

/**
 * @fn calResAllocLocal1D(CALresource* res, CALdevice dev, CALuint width, CALformat format, CALuint flags)
 *
 * @brief Allocate a 1D memory resource local to a device
 *
 * allocates memory resource local to a device <i>device</i> and returns
 * a resource handle in <i>*res</i> if successful. This memory is
 * structured as a 1 dimensional array of <i>width</i> elements with a <i>format</i>}.
 * The maximum values of <i>width</i> is available from the calDeviceGetInfo function.
 * The call returns CAL_RESULT_ERROR if requested memory was not available.
 *
 * @param res (out)   - returned resource handle. On error, res will be zero.
 * @param dev (in)    - device the resource should be local.
 * @param width (in)  - width of resource (in elements).
 * @param format (in) - format/type of each element of the resource.
 * @param flags (in) - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
CALAPI CALresult CALAPIENTRY calResAllocLocal1D(CALresource* res, CALdevice dev, CALuint width, CALformat format, CALuint flags);

/**
 * @fn calResAllocRemote1D(CALresource* res, CALdevice* dev, CALuint deviceCount, CALuint width, CALformat format, CALuint flags)
 *
 * @brief Allocate a 1D memory resource remote to a device
 *
 * allocates memory resource global to <i>devCount</i> number of devices
 * in <i>dev</i> array and returns a resource memory handle in <i>*res</i> if
 * successful. This memory resource is structured as a 1 dimensional
 * region of <i>width</i> elements with a <i>format</i>. The maximum values of
 * <i>width</i> is available from the calDeviceGetInfo function. The call returns
 * CAL_RESULT_ERROR if requested memory was not available.
 *
 * Currently only a single device is functional (<i>devCount</i> must be 1).
 *
 * @param res (out)   - returned resource handle. On error, res will be zero.
 * @param dev (in)    - device the resource should be local.
 * @param deviceCount (in) - number of devices in the device list.
 * @param width (in)  - width of resource (in elements).
 * @param format (in) - format/type of each element of the resource.
 * @param flags (in) - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
CALAPI CALresult CALAPIENTRY calResAllocRemote1D(CALresource* res, CALdevice* dev, CALuint deviceCount, CALuint width, CALformat format, CALuint flags);

/**
 * @fn calResFree(CALresource res)
 *
 * @brief Free a resource
 *
 * releases allocated memory resource. calResFree returns CAL_RESULT_BUSY if
 * the resources is in use by any context.
 *
 * @param res (in)   - resource handle to free.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResAllocLocal2D calResAllocRemote2D calResAllocLocal1D calResAllocRemote1D
 */
CALAPI CALresult CALAPIENTRY calResFree(CALresource res);

/**
 * @fn calResMap(CALvoid** pPtr, CALuint* pitch, CALresource res, CALuint flags)
 *
 * @brief Map memory to the CPU
 *
 *
 * returns a CPU accessible pointer to the memory surface in <i>**pPtr</i>
 * and the pitch in <i>*pitch</i>.  All memory resources are CPU accessible. It is an
 * error to call <i>calResMap</i> within a <i>calResMap</i> - <i>calResUnmap</i> pair
 * for the same <i>CALresource</i> memory resource handle.
 *
 * A mapped surface cannot be used as input or output of a calCtxRunProgram or calMemCopy.
 *
 * @param pPtr (out) - CPU pointer to the mapped resource. On error, pPtr will be zero.
 * @param pitch (out) - Pitch in elements of the resource. On error, pitch will be zero.
 * @param res (in) - resource handle to map
 * @param flags (in) - not used
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResUnmap
 */
CALAPI CALresult CALAPIENTRY calResMap(CALvoid** pPtr, CALuint* pitch, CALresource res, CALuint flags);

/**
 * @fn calResUnmap(CALresource res)
 *
 * @brief Unmap a CPU mapped resource.
 *
 * releases the address returned in <i>calResMap</i>. This should always be
 * paired with <i>calResMap</i>
 *
 * @param res (in) - resource handle to unmap
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResMap
 */
CALAPI CALresult CALAPIENTRY calResUnmap(CALresource res);


/*----------------------------------------------------------------------------
 * CAL Context Functions
 *----------------------------------------------------------------------------*/

/**
 * @fn calCtxCreate(CALcontext* ctx, CALdevice dev)
 *
 * @brief Create a CAL context on the specified device
 *
 * creates a context on a device. Multiple contexts can be created on
 * a single device.
 *
 * @param ctx (out) - handle of the newly created context. On error, ctx will be zero.
 * @param dev (in) - device handle to create the context on
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxDestroy
 */
CALAPI CALresult CALAPIENTRY calCtxCreate(CALcontext* ctx, CALdevice dev);

/**
 * @fn calCtxDestroy(CALcontext ctx)
 *
 * @brief Destroy a CAL context
 *
 * destroys a context. All current modules are unloaded and all CALmem objects
 * mapped to the context are released. This call should be paired with
 * <i>calCtxCreate</i>
 *
 * @param ctx (in) - context to destroy
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxCreate
 */
CALAPI CALresult CALAPIENTRY calCtxDestroy(CALcontext ctx);

/**
 * @fn calCtxGetMem(CALmem* mem, CALcontext ctx, CALresource res)
 *
 * @brief Map a resource to a context
 *
 * returns a memory handle in <i>*mem</i> for the resource surface <i>res</i>
 * for use by the context <i>ctx</i>.
 *
 * @param mem (out) - created memory handle. On error, mem will be zero.
 * @param ctx (in) - context in which resource is mapped
 * @param res (in) - resource to map to context
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxReleaseMem calCtxSetMem
 */
CALAPI CALresult CALAPIENTRY calCtxGetMem(CALmem* mem, CALcontext ctx, CALresource res);

/**
 * @fn calCtxReleaseMem(CALcontext ctx, CALmem mem)
 *
 * @brief Release a resource to context mapping
 *
 * releases memory handle <i>mem</i> that is obtained by <i>calCtxGetMem</i>.
 *
 * @param ctx (in) - context in which resource is mapped
 * @param mem (in) - memory handle to release
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxGetMem calCtxSetMem
 */
CALAPI CALresult CALAPIENTRY calCtxReleaseMem(CALcontext ctx, CALmem mem);

/**
 * @fn calCtxSetMem(CALcontext ctx, CALname name, CALmem mem)
 *
 * @brief Set memory used for kernel input or output
 *
 * sets a memory handle <i>mem</i> with the associated <i>name</i> in
 * the module to the context <i>ctx</i>. This can be input or output.
 *
 * @param ctx (in) - context to apply attachment.
 * @param name (in) - name to bind memory.
 * @param mem (in) - memory handle to apply.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxGetMem calCtxReleaseMem
 */
CALAPI CALresult CALAPIENTRY calCtxSetMem(CALcontext ctx, CALname name, CALmem mem);

/**
 * @fn calCtxRunProgram(CALevent* event, CALcontext ctx, CALfunc func, const CALdomain* domain)
 *
 * @brief Invoke the kernel over the specified domain.
 *
 *
 * issues a task to invoke the computation of the kernel identified by
 * <i>func</i> within a region <i>domain</i> on the context <i>ctx</i> and
 * returns an associated event token in <i>*event</i> with this task. This
 * method returns CAL_RESULT_ERROR if <i>func</i> is not found in the currently
 * loaded module. This method returns CAL_RESULT_ERROR, if any of the inputs,
 * input references, outputs and constant buffers associated with the kernel
 * are not setup. Completion of this event can be queried by the master process
 * using <i>calIsEventDone</i>
 *
 * Extended contextual information regarding a calCtxRunProgram failure
 * can be obtained with the calGetErrorString function.
 *
 * @param event (out) - event associated with RunProgram instance. On error, event will be zero.
 * @param ctx (in) - context.
 * @param func (in) - function to use as kernel.
 * @param domain (in) - domain over which kernel is applied.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxIsEventDone
 */
CALAPI CALresult CALAPIENTRY calCtxRunProgram(CALevent* event, CALcontext ctx, CALfunc func, const CALdomain* domain);

/**
 * @fn calCtxIsEventDone(CALcontext ctx, CALevent event)
 *
 * @brief Query to see if event has completed
 *
 *
 * is a mechanism for the master process to query if an event <i>event</i> on
 * context <i>ctx</i> from <i>calCtxRunProgram</i> or <i>calMemCopy</i> is
 * completed. This call also ensures that the commands associated with
 * the context are flushed.
 *
 * @param ctx (in) - context to query.
 * @param event (in) - event to query.
 *
 * @return Returns CAL_RESULT_OK if the event is complete, CAL_RESULT_PENDING if the event is
 * still being processed and CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxRunProgram
 */
CALAPI CALresult CALAPIENTRY calCtxIsEventDone(CALcontext ctx, CALevent event);

/**
 * @fn calCtxFlush(CALcontext ctx)
 *
 * @brief Flush any commands associated with the supplied context
 *
 * This call ensures that the commands associated with the
 * context are flushed.
 *
 * @param ctx (in) - context to flush.
 *
 * @return Returns CAL_RESULT_OK if the event is complete, CAL_RESULT_ERROR if
 * there was an error.
 *
 * @sa calCtxRunProgram calCtxIsEventDone
 */
CALAPI CALresult CALAPIENTRY calCtxFlush(CALcontext ctx);

/**
 * @fn calMemCopy(CALevent* event, CALcontext ctx, CALmem srcMem, CALmem dstMem, CALuint flags)
 *
 * @brief Copy srcMem to dstMem
 *
 * issues a task to copy data from a source memory handle to a
 * destination memory handle. This method returns CAL_RESULT_ERROR if the source
 * and destination memory have different memory formats or if the destination
 * memory handle is not as big in 2 dimensions as the source memory or
 * if the source and destination memory handles do not belong to the
 * context <i>ctx</i>. An event is associated with this task and is returned in
 * <i>*event</i> and completion of this event can be queried by the master
 * process using <i>calIsEventDone</i>. Data can be copied between memory
 * handles from remote system memory to device local memory, remote system
 * memory to remote system memory, device local memory to remote
 * system memory, device local memory to same device local memory, device
 * local memory to a different device local memory. The memory is copied by
 * the context <i>ctx</i>
 *
 * @param event (out) - event associated with Memcopy instance. On error, event will be zero.
 * @param ctx (in) - context to query.
 * @param srcMem (in) - source of the copy.
 * @param dstMem (in) - destination of the copy.
 * @param flags (in) - currently not used.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxRunProgram
 */
CALAPI CALresult CALAPIENTRY calMemCopy(CALevent* event, CALcontext ctx, CALmem srcMem, CALmem dstMem, CALuint flags);

/*----------------------------------------------------------------------------
 * CAL Image Functions
 *----------------------------------------------------------------------------*/

/**
 * @fn calImageRead(CALimage* image, const CALvoid* buffer, CALuint size)
 *
 * @brief Create a CALimage and serialize into it from the supplied buffer.
 *
 * Create a CALimage and populate it with information from the supplied buffer.
 *
 * @param image (out) - image created from serialization
 * @param buffer (in) - buffer to serialize from
 * @param size (in) - size of buffer
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 * @sa calImageFree
 */
CALAPI CALresult CALAPIENTRY calImageRead(CALimage *image, const CALvoid* buffer, CALuint size);

/**
 * @fn calImageFree(CALimage image)
 *
 * @brief Free the supplied CALimage.
 *
 * Free a calImage that was created with calImageRead.
 *
 * @param image (in) - image to free
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calImageRead
 */
CALAPI CALresult CALAPIENTRY calImageFree(CALimage image);

/*----------------------------------------------------------------------------
 * CAL Module Functions
 *----------------------------------------------------------------------------*/

/**
 * @fn calModuleLoad(CALmodule* module, CALcontext ctx, CALimage image)
 *
 * @brief Load a kernel image to a context
 *
 * creates a module from precompiled image <i>image</i>, loads the module
 * on the context and returns the loaded module in <i>*module</i>. This
 * method returns CAL_RESULT_ERROR if the module cannot be loaded onto the
 * processor. One of the reasons why a module cannot be loaded is if the
 * module does not have generated ISA for the hardware that it is loaded
 * onto. Multiple images can be loaded onto a single context at any single time.
 *
 * @param module (out) - handle to the loaded image. On error, module will be zero.
 * @param ctx (in) - context to load an image.
 * @param image (in) - raw image to load.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calModuleUnload calModuleGetEntry calModuleGetName
 */
CALAPI CALresult CALAPIENTRY calModuleLoad(CALmodule* module, CALcontext ctx, CALimage image);

/**
 * @fn calModuleUnload(CALcontext ctx, CALmodule module)
 *
 * @brief Unload a kernel image
 *
 * unloads the module from the context.
 *
 * @param ctx (in) - context.
 * @param module (in) - handle to the loaded image.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calModuleLoad calModuleGetEntry calModuleGetName
 */
CALAPI CALresult CALAPIENTRY calModuleUnload(CALcontext ctx, CALmodule module);

/**
 * @fn calModuleGetEntry(CALfunc* func, CALcontext ctx, CALmodule module, const CALchar* procName)
 *
 * @brief Retrieve a kernel function
 *
 * returns in <i>*func</i> the entry point to the kernel function named
 * <i>procName</i> from the module <i>module</i>. This method returns
 * CAL_RESULT_ERROR if the entry point <i>procName</i> is not found in the module.
 *
 * @param func (out) - handle to kernel function. On error, func will be zero.
 * @param ctx (in) - context.
 * @param module (in) - handle to the loaded image.
 * @param procName (in) - name of the function.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calModuleLoad calModuleUnload calModuleGetEntry
 */
CALAPI CALresult CALAPIENTRY calModuleGetEntry(CALfunc* func, CALcontext ctx, CALmodule module, const CALchar* procName);

/**
 * @fn calModuleGetName(CALname* name, CALcontext ctx, CALmodule module, const CALchar* varName)
 *
 * @brief Retrieve a kernel parameter by name
 *
 * returns in <i>*name</i> the handle to the module global variable named
 * <i>varName</i> that can be used to setup inputs and constant buffers to
 * the kernel computation. This method returns CAL_RESULT_ERROR if the variable
 * <i>varName</i> is not found in the module.
 *
 * @param name (out) - handle to name symbol. On error, name will be zero.
 * @param ctx (in) - context.
 * @param module (in) - handle to the loaded image.
 * @param varName (in) - name of the input or output.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calModuleLoad calModuleUnload calModuleGetEntry
 */
CALAPI CALresult CALAPIENTRY calModuleGetName(CALname* name, CALcontext ctx, CALmodule module, const CALchar* varName);

/*----------------------------------------------------------------------------
 * CAL Error/Debug Helper Functions
 *----------------------------------------------------------------------------*/
/**
 * @fn calGetErrorString(void)
 *
 * @brief Return details about current error state
 *
 * calGetErrorString returns a text string containing details about the last
 * returned error condition. Calling calGetErrorString does not effect the
 * error state.
 *
 * @return Returns a null terminated string detailing the error condition
 *
 * @sa calInit calShutdown
 */
CALAPI const CALchar* CALAPIENTRY calGetErrorString(void);

/**
 * @fn calCtxRunProgramGrid(CALevent* event, CALcontext ctx, CALprogramGrid* pProgramGrid)
 *
 * @brief Invoke the kernel over the specified domain.
 *
 *
 * issues a task to invoke the computation of the kernel identified by
 * <i>func</i> within a region <i>domain</i> on the context <i>ctx</i> and
 * returns an associated event token in <i>*event</i> with this task. This
 * method returns CAL_RESULT_ERROR if <i>func</i> is not found in the currently
 * loaded module. This method returns CAL_RESULT_ERROR, if any of the inputs,
 * input references, outputs and constant buffers associated with the kernel
 * are not setup. Completion of this event can be queried by the master process
 * using <i>calIsEventDone</i>
 *
 * Extended contextual information regarding a calCtxRunProgram failure
 * can be obtained with the calGetErrorString function.
 *
 * @param event (out) - event associated with RunProgram instance. On error, event will be zero.
 * @param ctx (in) - context.
 * @param pProgramGrid (in) - description of program information to get kernel and thread counts.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxIsEventDone
 */
CALAPI CALresult CALAPIENTRY calCtxRunProgramGrid(CALevent* event,
                                                  CALcontext ctx,
                                                  CALprogramGrid* pProgramGrid);

/**
 * @fn calModuleGetFuncInfo(CALfuncInfo* pInfo, CALcontext ctx, CALmodule module, CALfunc func)
 *
 * @brief Retrieve information regarding the named func in the
 * named module.
 *
 * returns in <i>*info</i> the information regarding the func.
 * This method returns CAL_RESULT_NOT_INITIALIZED if CAL is not
 * initialied.
 * This method returns CAL_RESULT_INVALID_PARAMETER if info is
 * NULL.
 * This method returns CAL_RESULT_BAD_HANDLE if ctx is invalid
 * or module is not loaded or func is not found.
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param pInfo (out) - pointer to CALmoduleInfo output
 *              structure.
 * @param ctx (in) - context.
 * @param module (in) - handle to the loaded image.
 * @param func (in) - name of the function.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_NOT_INITIALIZED,
 *         CAL_RESULT_INVALID_PARAMETER, CAL_RESULT_BAD_HANDLE,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
CALAPI CALresult CALAPIENTRY calModuleGetFuncInfo(CALfuncInfo* pInfo,
                     CALcontext   ctx,
                     CALmodule    module,
                     CALfunc      func);

/**
 * @fn calCtxRunProgramGridArray(CALevent* event, CALcontext ctx, CALprogramGridArray* pGridArray)
 *
 * @brief Invoke the kernel array over the specified domain(s).
 *
 *
 * issues a task to invoke the computation of the kernel arrays identified by
 * <i>func</i> within a region <i>domain</i> on the context <i>ctx</i> and
 * returns an associated event token in <i>*event</i> with this task. This
 * method returns CAL_RESULT_ERROR if <i>func</i> is not found in the currently
 * loaded module. This method returns CAL_RESULT_ERROR, if any of the inputs,
 * input references, outputs and constant buffers associated with the kernel
 * are not setup. Completion of this event can be queried by the master process
 * using <i>calIsEventDone</i>
 *
 * Extended contextual information regarding a calCtxRunProgram failure
 * can be obtained with the calGetErrorString function.
 *
 * @param event (out) - event associated with RunProgram instance. On error, event will be zero.
 * @param ctx (in) - context.
 * @param pGridArray (in) - array containing kernel programs and grid information.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calCtxIsEventDone
 */
CALAPI CALresult CALAPIENTRY calCtxRunProgramGridArray(CALevent* event,
                                                  CALcontext ctx,
                                                  CALprogramGridArray* pGridArray);
#ifdef __cplusplus
}      /* extern "C" { */
#endif


#endif /* __CAL_H__ */



