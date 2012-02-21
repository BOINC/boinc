.SUFFIXES : .cl

P4_ROOT ?= ${HOME}/perforce/

# detect OS
OSUPPER = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
OSLOWER = $(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])
# 'linux' is output for Linux system, 'darwin' for OS X
DARWIN = $(strip $(findstring DARWIN, $(OSUPPER)))
ifneq ($(DARWIN),)
   SNOWLEOPARD = $(strip $(findstring 10.6, $(shell egrep "<string>10\.6" /System/Library/CoreServices/SystemVersion.plist)))
endif

# detect if 32 bit or 64 bit system
HP_64 =	$(shell uname -m | grep 64)
OSARCH= $(shell uname -m)

# BOINC directory
BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

# Basic directory setup for SDK
# (override directories only if they are not already defined)
SRCDIR     ?= 
ROOTDIR    ?= /home/tuanle/NVIDIA_GPU_Computing_SDK/
ROOTOBJDIR ?= obj
LIBDIR     := $(ROOTDIR)/shared/lib/
SHAREDDIR  := $(ROOTDIR)/shared/
OCLROOTDIR := $(ROOTDIR)/OpenCL/
OCLCOMMONDIR ?= $(OCLROOTDIR)/common/
OCLBINDIR ?= $(BOINC_DIR)/samples/nvopencl/
BINDIR     ?= $(OCLBINDIR)/$(OSLOWER)
OCLLIBDIR     := $(OCLCOMMONDIR)/lib
INCDIR	?= $(ROOTDIR)/OpenCL/common/inc/

# Compilers
CXX        := g++
CC         := gcc
# MODIFIED - add -lpthread to fix undefined reference in function start_timer_thread() in boinc_api.cpp
LINK       := g++ -fPIC -lpthread

# Includes
INCLUDES  += -I$(INCDIR) -I$(OCLCOMMONDIR)/inc -I$(SHAREDDIR)/inc -I $(BOINC_API_DIR) -I $(BOINC_LIB_DIR) -I $(BOINC_DIR)

ifeq "$(strip $(HP_64))" ""
	MACHINE := 32
	USRLIBDIR := -L/usr/lib/
else
	MACHINE := 64
	USRLIBDIR := -L/usr/lib64/
endif


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
LIB_ARCH        := $(OSARCH)

# Determining the necessary Cross-Compilation Flags
# 32-bit OS, but we target 64-bit cross compilation
ifeq ($(x86_64),1)
    LIB_ARCH         = x86_64

    ifneq ($(DARWIN),)
         CXX_ARCH_FLAGS += -arch x86_64
    else
         CXX_ARCH_FLAGS += -m64
    endif
else
# 64-bit OS, and we target 32-bit cross compilation
    ifeq ($(i386),1)
        LIB_ARCH         = i386

        ifneq ($(DARWIN),)
             CXX_ARCH_FLAGS += -arch i386
        else
             CXX_ARCH_FLAGS += -m32
        endif
    else
        ifneq ($(SNOWLEOPARD),)
             CXX_ARCH_FLAGS += -m32 -arch i386
             LIB_ARCH        = i386
        else
             ifeq "$(strip $(HP_64))" ""
                LIB_ARCH        = i386
             else
                LIB_ARCH        = x86_64
             endif
        endif
    endif
endif

# Compiler-specific flags
CXXFLAGS  := $(CXXWARN_FLAGS) $(CXX_ARCH_FLAGS)
CFLAGS    := $(CWARN_FLAGS) $(CXX_ARCH_FLAGS)
LINK      += $(CXX_ARCH_FLAGS)

# Common flags
COMMONFLAGS += $(INCLUDES) -DUNIX

# Add Mac Flags
ifneq ($(DARWIN),)
	COMMONFLAGS += -DMAC
endif

# Debug/release configuration
ifeq ($(dbg),1)
	COMMONFLAGS += -g
	BINSUBDIR   := debug
	LIBSUFFIX   := D
else 
	COMMONFLAGS += -O3 
	BINSUBDIR   := release
	LIBSUFFIX   :=
	CXXFLAGS    += -fno-strict-aliasing
	CFLAGS      += -fno-strict-aliasing
endif


# OpenGL is used or not (if it is used, then it is necessary to include GLEW)
ifeq ($(USEGLLIB),1)

	ifneq ($(DARWIN),)
		OPENGLLIB := -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL -lGLU $(SHAREDDIR)/lib/$(OSLOWER)/libGLEW.a
	else
		OPENGLLIB := -lGL -lGLU -lX11 -lXmu
		ifeq "$(strip $(HP_64))" ""
			OPENGLLIB += -lGLEW -L/usr/X11R6/lib
		else
			OPENGLLIB += -lGLEW_x86_64 -L/usr/X11R6/lib64
		endif
	endif

	CUBIN_ARCH_FLAG := -m64
endif

ifeq ($(USEGLUT),1)
	ifneq ($(DARWIN),)
		OPENGLLIB += -framework GLUT
		INCLUDES += -I/System/Library/Frameworks/OpenGL.framework/Headers
	else
		OPENGLLIB += -lglut
	endif
endif

# Libs
ifneq ($(DARWIN),)
   LIB       := -L${OCLLIBDIR} -L$(LIBDIR) -L$(SHAREDDIR)/lib/$(OSLOWER) 
   LIB += -framework OpenCL -framework OpenGL ${OPENGLLIB} -framework AppKit ${ATF} ${LIB} 
else
   LIB       := ${USRLIBDIR} -L${OCLLIBDIR} -L$(LIBDIR) -L$(SHAREDDIR)/lib/$(OSLOWER) 
   # MODIFIED - link boinc lib and boinc api
   LIB += -lOpenCL ${OPENGLLIB} ${LIB} -lboinc_api -L$(BOINC_API_DIR) -lboinc -L$(BOINC_LIB_DIR)
endif


# Lib/exe configuration
ifneq ($(STATIC_LIB),)
	TARGETDIR := $(OCLLIBDIR)
	TARGET   := $(subst .a,_$(LIB_ARCH)$(LIBSUFFIX).a,$(OCLLIBDIR)/$(STATIC_LIB))
	LINKLINE  = ar qv $(TARGET) $(OBJS) 
else
	LIB += -loclUtil_$(LIB_ARCH)$(LIBSUFFIX) -lshrutil_$(LIB_ARCH)$(LIBSUFFIX)
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

# Add common flags
CXXFLAGS  += $(COMMONFLAGS)
CFLAGS    += $(COMMONFLAGS)


################################################################################
# Set up object files
################################################################################
OBJDIR := $(ROOTOBJDIR)/$(BINSUBDIR)
OBJS +=  $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(notdir $(CCFILES)))
OBJS +=  $(patsubst %.c,$(OBJDIR)/%.c.o,$(notdir $(CFILES)))

################################################################################
# Rules
################################################################################
$(OBJDIR)/%.c.o : $(SRCDIR)%.c $(C_DEPS)
	$(VERBOSE)$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.cpp.o : $(SRCDIR)%.cpp $(C_DEPS)
	$(VERBOSE)$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): makedirectories $(OBJS) Makefile
	$(VERBOSE)$(LINKLINE)

makedirectories:
	$(VERBOSE)mkdir -p $(LIBDIR)
	$(VERBOSE)mkdir -p $(OBJDIR)
	$(VERBOSE)mkdir -p $(TARGETDIR)


tidy :
	$(VERBOSE)find . | egrep "#" | xargs rm -f
	$(VERBOSE)find . | egrep "\~" | xargs rm -f

clean : tidy
	$(VERBOSE)rm -f $(OBJS)
	$(VERBOSE)rm -f $(TARGET)

clobber : clean
	$(VERBOSE)rm -rf $(ROOTOBJDIR)
	$(VERBOSE)find $(TARGETDIR) | egrep "ptx" | xargs rm -f
	$(VERBOSE)find $(TARGETDIR) | egrep "txt" | xargs rm -f
	$(VERBOSE)rm -f $(TARGETDIR)/samples.list
