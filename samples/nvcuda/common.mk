.SUFFIXES : .cu .cu_dbg.o .c_dbg.o .cpp_dbg.o .cu_rel.o .c_rel.o .cpp_rel.o .cubin .ptx

# Add new SM Versions here as devices with new Compute Capability are released
SM_VERSIONS   := 10 11 12 13 20

CUDA_INSTALL_PATH ?= /usr/local/cuda

ifdef cuda-install
	CUDA_INSTALL_PATH := $(cuda-install)
endif

# detect OS
OSUPPER = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
OSLOWER = $(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])

# 'linux' is output for Linux system, 'darwin' for OS X
DARWIN = $(strip $(findstring DARWIN, $(OSUPPER)))
ifneq ($(DARWIN),)
   SNOWLEOPARD = $(strip $(findstring 10.6, $(shell grep -E "<string>10\.6" /System/Library/CoreServices/SystemVersion.plist)))
endif

# detect 32-bit or 64-bit platform
HP_64 = $(shell uname -m | grep 64)
OSARCH= $(shell uname -m)

# Basic directory setup for SDK
# (override directories only if they are not already defined)
SRCDIR     ?=
ROOTDIR    ?= /home/tuanle/NVIDIA_GPU_Computing_SDK

# ROOTDIR/bin is the directory where executable file will be put in

ROOTBINDIR ?= ../../samples/nvcuda
BINDIR     ?= $(ROOTBINDIR)/$(OSLOWER)
ROOTOBJDIR ?= obj

# BOINC directory
BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

# Directory for cutil_i386 as well as other utils while linking (-lcutil_i386)
LIBDIR     := $(ROOTDIR)/C/lib
COMMONDIR  := $(ROOTDIR)/C/common
SHAREDDIR  := $(ROOTDIR)/shared

# Compilers
NVCC       := $(CUDA_INSTALL_PATH)/bin/nvcc
CXX        := g++-4.3
CC         := gcc-4.3
LINK       := g++-4.3 -fPIC

# Includes
INCLUDES  += -I. -I$(CUDA_INSTALL_PATH)/include -I$(COMMONDIR)/inc -I$(SHAREDDIR)/inc -I $(BOINC_API_DIR) -I $(BOINC_LIB_DIR) -I $(BOINC_DIR)

# Warning flags
CXXWARN_FLAGS := \
	-W -Wall \
	-Wimplicit \
	-Wswitch \
	-Wformat \
	-Wchar-subscripts \
	-Wparentheses \
	-Wmultichar \
	-Wtrigraphs \
	-Wpointer-arith \
	-Wcast-align \
	-Wreturn-type \
	-Wno-unused-function \
	$(SPACE)

CWARN_FLAGS := $(CXXWARN_FLAGS) \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wnested-externs \
	-Wmain \

# architecture flag for nvcc and gcc compilers build
CUBIN_ARCH_FLAG :=
CXX_ARCH_FLAGS  :=
NVCCFLAGS       :=
LIB_ARCH        := $(OSARCH)

# Determining the necessary Cross-Compilation Flags
# 32-bit OS, but we target 64-bit cross compilation
ifeq ($(x86_64),1)
    NVCCFLAGS       += -m64
    LIB_ARCH         = x86_64
    CUDPPLIB_SUFFIX  = x86_64

    ifneq ($(DARWIN),)
         CXX_ARCH_FLAGS += -arch x86_64
    else
         CXX_ARCH_FLAGS += -m64
    endif
else
# 64-bit OS, and we target 32-bit cross compilation
    ifeq ($(i386),1)
        NVCCFLAGS       += -m32
        LIB_ARCH         = i386
        CUDPPLIB_SUFFIX  = i386

        ifneq ($(DARWIN),)
             CXX_ARCH_FLAGS += -arch i386
        else
             CXX_ARCH_FLAGS += -m32
        endif
    else
        ifneq ($(SNOWLEOPARD),)
             NVCCFLAGS += -m32
             CXX_ARCH_FLAGS += -m32 -arch i386
             LIB_ARCH        = i386
             CUDPPLIB_SUFFIX = i386
        else
             ifeq "$(strip $(HP_64))" ""
                LIB_ARCH        = i386
                CUDPPLIB_SUFFIX = i386
             else
                LIB_ARCH        = x86_64
                CUDPPLIB_SUFFIX = x86_64
             endif
        endif
    endif
endif

# Compiler-specific flags (by default, we always use sm_10 and sm_20), unless we use the SMVERSION template
GENCODE_SM10 := -gencode=arch=compute_10,code=\"sm_10,compute_10\"
GENCODE_SM20 := -gencode=arch=compute_20,code=\"sm_20,compute_20\"

CXXFLAGS  += $(CXXWARN_FLAGS) $(CXX_ARCH_FLAGS)
CFLAGS    += $(CWARN_FLAGS) $(CXX_ARCH_FLAGS)
LINKFLAGS +=
LINK      += $(LINKFLAGS) $(CXX_ARCH_FLAGS)

# This option for Mac allows CUDA applications to work without requiring to set DYLD_LIBRARY_PATH
ifneq ($(DARWIN),)
   LINK += -Xlinker -rpath $(CUDA_INSTALL_PATH)/lib
endif

# Common flags
COMMONFLAGS += $(INCLUDES) -DUNIX

# Debug/release configuration
ifeq ($(dbg),1)
	COMMONFLAGS += -g
	NVCCFLAGS   += -D_DEBUG
	CXXFLAGS    += -D_DEBUG
	CFLAGS      += -D_DEBUG
	BINSUBDIR   := debug
	LIBSUFFIX   := D
else
	COMMONFLAGS += -O3
	BINSUBDIR   := release
	LIBSUFFIX   :=
	NVCCFLAGS   += --compiler-options -fno-strict-aliasing
	CXXFLAGS    += -fno-strict-aliasing
	CFLAGS      += -fno-strict-aliasing
endif

# architecture flag for cubin build
CUBIN_ARCH_FLAG :=

# OpenGL is used or not (if it is used, then it is necessary to include GLEW)
ifeq ($(USEGLLIB),1)
    ifneq ($(DARWIN),)
        OPENGLLIB := -L/System/Library/Frameworks/OpenGL.framework/Libraries
        OPENGLLIB += -lGL -lGLU $(COMMONDIR)/lib/$(OSLOWER)/libGLEW.a
    else
# this case for linux platforms
	OPENGLLIB := -lGL -lGLU -lX11 -lXi -lXmu
# check if x86_64 flag has been set, otherwise, check HP_64 is i386/x86_64
        ifeq ($(x86_64),1)
	       OPENGLLIB += -lGLEW_x86_64 -L/usr/X11R6/lib64
        else
             ifeq ($(i386),)
                 ifeq "$(strip $(HP_64))" ""
	             OPENGLLIB += -lGLEW -L/usr/X11R6/lib
                 else
	             OPENGLLIB += -lGLEW_x86_64 -L/usr/X11R6/lib64
                 endif
             endif
        endif
# check if i386 flag has been set, otehrwise check HP_64 is i386/x86_64
        ifeq ($(i386),1)
	       OPENGLLIB += -lGLEW -L/usr/X11R6/lib
        else
             ifeq ($(x86_64),)
                 ifeq "$(strip $(HP_64))" ""
	             OPENGLLIB += -lGLEW -L/usr/X11R6/lib
                 else
	             OPENGLLIB += -lGLEW_x86_64 -L/usr/X11R6/lib64
                 endif
             endif
        endif
    endif
endif

ifeq ($(USEGLUT),1)
    ifneq ($(DARWIN),)
	OPENGLLIB += -framework GLUT
    else
        ifeq ($(x86_64),1)
	     OPENGLLIB += -lglut -L/usr/lib64
        endif
        ifeq ($(i386),1)
	     OPENGLLIB += -lglut -L/usr/lib
        endif

        ifeq ($(x86_64),)
            ifeq ($(i386),)
	        OPENGLLIB += -lglut
            endif
        endif
    endif
endif

ifeq ($(USEPARAMGL),1)
	PARAMGLLIB := -lparamgl_$(LIB_ARCH)$(LIBSUFFIX)
endif

ifeq ($(USERENDERCHECKGL),1)
	RENDERCHECKGLLIB := -lrendercheckgl_$(LIB_ARCH)$(LIBSUFFIX)
endif

ifeq ($(USECUDPP), 1)
    CUDPPLIB := -lcudpp_$(CUDPPLIB_SUFFIX)$(LIBSUFFIX)

    ifeq ($(emu), 1)
        CUDPPLIB := $(CUDPPLIB)_emu
    endif
endif

ifeq ($(USENVCUVID), 1)
     ifneq ($(DARWIN),)
         NVCUVIDLIB := -L$(ROOTDIR)/C/common/lib/darwin -lnvcuvid
     endif
endif

# Libs
ifneq ($(DARWIN),)
    LIB       := -L$(CUDA_INSTALL_PATH)/lib -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib $(NVCUVIDLIB)
else
  ifeq "$(strip $(HP_64))" ""
    ifeq ($(x86_64),1)
       LIB       := -L$(CUDA_INSTALL_PATH)/lib64 -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    else
       LIB       := -L$(CUDA_INSTALL_PATH)/lib -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    endif
  else
    ifeq ($(i386),1)
       LIB       := -L$(CUDA_INSTALL_PATH)/lib -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    else
       LIB       := -L$(CUDA_INSTALL_PATH)/lib64 -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    endif
  endif
endif

# If dynamically linking to CUDA and CUDART, we exclude the libraries from the LIB
ifeq ($(USECUDADYNLIB),1)
     LIB += ${OPENGLLIB} $(PARAMGLLIB) $(RENDERCHECKGLLIB) $(CUDPPLIB) ${LIB} -ldl -rdynamic
else
# static linking, we will statically link against CUDA and CUDART
  ifeq ($(USEDRVAPI),1)
     LIB += -lcuda   ${OPENGLLIB} $(PARAMGLLIB) $(RENDERCHECKGLLIB) $(CUDPPLIB) ${LIB}
  else
     ifeq ($(emu),1)
         LIB += -lcudartemu
     else
         LIB += -lcudart
     endif
     LIB += ${OPENGLLIB} $(PARAMGLLIB) $(RENDERCHECKGLLIB) $(CUDPPLIB) ${LIB}
  endif
endif

ifeq ($(USECUFFT),1)
  ifeq ($(emu),1)
    LIB += -lcufftemu
  else
    LIB += -lcufft
  endif
endif

ifeq ($(USECUBLAS),1)
  ifeq ($(emu),1)
    LIB += -lcublasemu
  else
    LIB += -lcublas
  endif
endif

# Lib/exe configuration
ifneq ($(STATIC_LIB),)
	TARGETDIR := $(LIBDIR)
	TARGET   := $(subst .a,_$(LIB_ARCH)$(LIBSUFFIX).a,$(LIBDIR)/$(STATIC_LIB))
	LINKLINE  = ar rucv $(TARGET) $(OBJS)
else
	ifneq ($(OMIT_CUTIL_LIB),1)

# MODIFIED add -lboinc and -lboinc_api below
		LIB += -lcutil_$(LIB_ARCH)$(LIBSUFFIX) -lshrutil_$(LIB_ARCH)$(LIBSUFFIX) -lboinc_api -L$(BOINC_API_DIR) -lboinc -L$(BOINC_LIB_DIR)
	endif
	# Device emulation configuration
	ifeq ($(emu), 1)
		NVCCFLAGS   += -deviceemu
		CUDACCFLAGS +=
		BINSUBDIR   := emu$(BINSUBDIR)
		# consistency, makes developing easier
		CXXFLAGS		+= -D__DEVICE_EMULATION__
		CFLAGS			+= -D__DEVICE_EMULATION__
	endif
	TARGETDIR := $(BINDIR)/$(BINSUBDIR)
	TARGET    := $(TARGETDIR)/$(EXECUTABLE)
	LINKLINE  = $(LINK) -o $(TARGET) $(OBJS) $(LIB)
endif

# check if verbose
ifeq ($(verbose), 1)
	VERBOSE :=
else
	VERBOSE := @
endif

################################################################################
# Check for input flags and set compiler flags appropriately
################################################################################
ifeq ($(fastmath), 1)
	NVCCFLAGS += -use_fast_math
endif

ifeq ($(keep), 1)
	NVCCFLAGS += -keep
	NVCC_KEEP_CLEAN := *.i* *.cubin *.cu.c *.cudafe* *.fatbin.c *.ptx
endif

ifdef maxregisters
	NVCCFLAGS += -maxrregcount $(maxregisters)
endif

# Add cudacc flags
NVCCFLAGS += $(CUDACCFLAGS)

# Add common flags
NVCCFLAGS += $(COMMONFLAGS)
CXXFLAGS  += $(COMMONFLAGS)
CFLAGS    += $(COMMONFLAGS)

# use gcc-4.3
NVCCFLAGS+=--compiler-bindir=${HOME}/NVIDIA_GPU_Computing_SDK/C/mygcc

ifeq ($(nvcc_warn_verbose),1)
	NVCCFLAGS += $(addprefix --compiler-options ,$(CXXWARN_FLAGS))
	NVCCFLAGS += --compiler-options -fno-strict-aliasing
endif

################################################################################
# Set up object files
################################################################################
OBJDIR := $(ROOTOBJDIR)/$(LIB_ARCH)/$(BINSUBDIR)
OBJS +=  $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(notdir $(CCFILES)))
OBJS +=  $(patsubst %.c,$(OBJDIR)/%.c.o,$(notdir $(CFILES)))
OBJS +=  $(patsubst %.cu,$(OBJDIR)/%.cu.o,$(notdir $(CUFILES)))

################################################################################
# Set up cubin output files
################################################################################
CUBINDIR := $(SRCDIR)data
CUBINS +=  $(patsubst %.cu,$(CUBINDIR)/%.cubin,$(notdir $(CUBINFILES)))

################################################################################
# Set up PTX output files
################################################################################
PTXDIR := $(SRCDIR)data
PTXBINS +=  $(patsubst %.cu,$(PTXDIR)/%.ptx,$(notdir $(PTXFILES)))

################################################################################
# Rules
################################################################################
$(OBJDIR)/%.c.o : $(SRCDIR)%.c $(C_DEPS)
	$(VERBOSE)$(CC) $(CFLAGS) -o $@ -c $< -pthread \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a

$(OBJDIR)/%.cpp.o : $(SRCDIR)%.cpp $(C_DEPS)
	$(VERBOSE)$(CXX) $(CXXFLAGS) -o $@ -c $< -pthread \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a

# Default arch includes gencode for sm_10, sm_20, and other archs from GENCODE_ARCH declared in the makefile
$(OBJDIR)/%.cu.o : $(SRCDIR)%.cu $(CU_DEPS)
	$(VERBOSE)$(NVCC) $(GENCODE_SM10) $(GENCODE_ARCH) $(GENCODE_SM20) $(NVCCFLAGS) $(SMVERSIONFLAGS) -o $@ -c $<

# Default arch includes gencode for sm_10, sm_20, and other archs from GENCODE_ARCH declared in the makefile
$(CUBINDIR)/%.cubin : $(SRCDIR)%.cu cubindirectory
	$(VERBOSE)$(NVCC) $(GENCODE_SM10) $(GENCODE_ARCH) $(GENCODE_SM20) $(CUBIN_ARCH_FLAG) $(NVCCFLAGS) $(SMVERSIONFLAGS) -o $@ -cubin $<

$(PTXDIR)/%.ptx : $(SRCDIR)%.cu ptxdirectory
	$(VERBOSE)$(NVCC) $(CUBIN_ARCH_FLAG) $(NVCCFLAGS) $(SMVERSIONFLAGS) -o $@ -ptx $<

#
# The following definition is a template that gets instantiated for each SM
# version (sm_10, sm_13, etc.) stored in SMVERSIONS.  It does 2 things:
# 1. It adds to OBJS a .cu_sm_XX.o for each .cu file it finds in CUFILES_sm_XX.
# 2. It generates a rule for building .cu_sm_XX.o files from the corresponding
#    .cu file.
#
# The intended use for this is to allow Makefiles that use common.mk to compile
# files to different Compute Capability targets (aka SM arch version).  To do
# so, in the Makefile, list files for each SM arch separately, like so:
# This will be used over the default rule abov
#
# CUFILES_sm_10 := mycudakernel_sm10.cu app.cu
# CUFILES_sm_12 := anothercudakernel_sm12.cu
#
define SMVERSION_template
#OBJS += $(patsubst %.cu,$(OBJDIR)/%.cu_$(1).o,$(notdir $(CUFILES_$(1))))
OBJS += $(patsubst %.cu,$(OBJDIR)/%.cu_$(1).o,$(notdir $(CUFILES_sm_$(1))))
$(OBJDIR)/%.cu_$(1).o : $(SRCDIR)%.cu $(CU_DEPS)
#	$(VERBOSE)$(NVCC) -o $$@ -c $$< $(NVCCFLAGS)  $(1)
	$(VERBOSE)$(NVCC) -gencode=arch=compute_$(1),code=\"sm_$(1),compute_$(1)\" $(GENCODE_SM20) -o $$@ -c $$< $(NVCCFLAGS)
endef

# This line invokes the above template for each arch version stored in
# SM_VERSIONS.  The call funtion invokes the template, and the eval
# function interprets it as make commands.
$(foreach smver,$(SM_VERSIONS),$(eval $(call SMVERSION_template,$(smver))))

$(TARGET): makedirectories $(OBJS) $(CUBINS) $(PTXBINS) Makefile
	$(VERBOSE)$(LINKLINE)

cubindirectory:
	$(VERBOSE)mkdir -p $(CUBINDIR)

ptxdirectory:
	$(VERBOSE)mkdir -p $(PTXDIR)

makedirectories:
	$(VERBOSE)mkdir -p $(LIBDIR)
	$(VERBOSE)mkdir -p $(OBJDIR)
	$(VERBOSE)mkdir -p $(TARGETDIR)


tidy :
	$(VERBOSE)find . | grep -E "#" | xargs rm -f
	$(VERBOSE)find . | grep -E "\~" | xargs rm -f

clean : tidy
	$(VERBOSE)rm -f $(OBJS)
	$(VERBOSE)rm -f $(CUBINS)
	$(VERBOSE)rm -f $(PTXBINS)
	$(VERBOSE)rm -f $(TARGET)
	$(VERBOSE)rm -f $(NVCC_KEEP_CLEAN)
	$(VERBOSE)rm -f $(ROOTBINDIR)/$(OSLOWER)/$(BINSUBDIR)/*.ppm
	$(VERBOSE)rm -f $(ROOTBINDIR)/$(OSLOWER)/$(BINSUBDIR)/*.pgm
	$(VERBOSE)rm -f $(ROOTBINDIR)/$(OSLOWER)/$(BINSUBDIR)/*.bin
	$(VERBOSE)rm -f $(ROOTBINDIR)/$(OSLOWER)/$(BINSUBDIR)/*.bmp

clobber : clean
	$(VERBOSE)rm -rf $(ROOTOBJDIR)
