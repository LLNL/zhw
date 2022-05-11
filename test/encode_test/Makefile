#
# $Id: Makefile,v 2.2 2022/05/05 18:54:00 scott, michael Stab $
#
# Description: Makefile for test program
#
# $Log: Makefile,v $
#

ifdef D
  SEP := ,
  DEFS += $(patsubst %,-D%,$(subst $(SEP), ,$(D)))
endif

TARGET = encode_test
SRC = ../../src
MODULES = encode_test
#RUN_ARGS = -arg1 -arg2

VERSION = 1.0
LABEL = V$(subst .,_,$(VERSION))
#DEFS += -DVERSION=$(VERSION)

# SystemC specific
DEFS += -DTIMEOFDAY
SCDIR = src/systemc-2.3.2
LDFLAGS += -L$(HOME)/$(SCDIR)/objdir/src/.libs
ifeq ($(findstring c++11,$(CXXFLAGS)),)
  CXXFLAGS += -std=c++11
endif
SRC += $(HOME)/$(SCDIR)/src
# squelch warning from SystemC sc_bit_proxies.h
CFLAGS += -Wno-strict-overflow
# application build -std=option must match SystemC library build
LDLIBS += -lsystemc -lpthread

# ZFP specific
ZFPDIR = src/zfp
# ZFPDIR = work/helio/zfps3
ifneq ($(findstring ZCHK,$(DEFS)),)
  SRC += . $(HOME)/$(ZFPDIR)/include
  MODULES += zbatch
  LDFLAGS += -L$(HOME)/$(ZFPDIR)/lib
  LDLIBS += -lzfp -lm
endif

OBJECTS = $(addsuffix .o,$(MODULES))
VPATH = $(subst ' ',:,$(SRC))

OPT ?= -O3
#OPT += -ftree-vectorize -ffast-math
ifdef OMP
  OPT += -fopenmp
endif
#MACH = -march=core2 -mfpmath=sse
CPPFLAGS += -MMD $(DEFS)
CPPFLAGS += $(patsubst %,-I%,$(SRC))
CFLAGS += $(MACH) $(OPT) -Wall
CXXFLAGS += $(CFLAGS)
#LDFLAGS += -static
#LDLIBS += -lrt

# Cancel version control implicit rules
%:: %,v
%:: RCS/%
%:: RCS/%,v
%:: s.%
%:: SCCS/s.%
# Delete default suffixes
.SUFFIXES:
# Define suffixes of interest
.SUFFIXES: .o .c .cc .cpp .h .hpp .d .mak

.PHONY: all
all: $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET) $(RUN_ARGS)

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.d) $(TARGET) makeflags

.PHONY: vars
vars:
	@echo TARGET: $(TARGET)
	@echo VERSION: $(VERSION)
	@echo DEFS: $(DEFS)
	@echo SRC: $(SRC)
	@echo OBJECTS: $(OBJECTS)
	@echo MAKEFILE_LIST: $(MAKEFILE_LIST)

.PHONY: help
help:
	@echo "Make targets (make <targ>):"
	@echo "  all    build the target"
	@echo "  run    run the target"
	@echo "  clean  delete target build products"
	@echo "  vars   show makefile variables"
	@echo "Compile options through define (make D=DEF1=m,DEF2=n,...):"
	@echo "  DATAW=<int> bit width of encoded data stream"
	@echo "  DIMS=<int>  number of dimensions"
	@echo "  PREC=<int>  precison of floating-point numbers"
	@echo "  VCD         output a value change dump (VCD) file"
	@echo "  ZCHK        use ZFP lib for checking encoded blocks"
	@echo "  P_DATA      print data buffers, uncompressed and compressed"
	@echo "  P_IO        print I/O, time stamps and data"
	@echo "Makefile variables (make VAR1=x VAR2=y):"
	@echo "  OPT=<str>   optimization flags"
	@echo "  RUN_ARGS=\"<arg1> <arg2> ...\" target run time arguments"

$(TARGET): $(OBJECTS)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(OBJECTS): $(MAKEFILE_LIST) # rebuild if MAKEFILEs change

$(OBJECTS): makeflags # rebuild if MAKEFLAGS change
# Select only command line variables
cvars = _$(strip $(foreach flag,$(MAKEFLAGS),$(if $(findstring =,$(flag)),$(flag),)))_
makeflags: FORCE
	@[ "$(if $(wildcard $@),$(shell cat $@),)" = "$(cvars)" ] || echo $(cvars)> $@
FORCE: ;

# Establish module specific dependencies
-include $(OBJECTS:.o=.d)
