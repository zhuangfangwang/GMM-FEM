CC=g++
LD=g++

# DEBUG = 0
# PROFILE = 0

BUILDDIR = ./build
SRCDIR = ./src
INCLUDEDIR = ./src
TESTDIR = ./test
MEXDIR = ./mex
BINDIR = ./bin

CPPFLAGS=-I$(INCLUDEDIR) --std=c++11
ifdef DEBUG
CPPFLAGS:=$(CPPFLAGS) -g -O0 -DMAS_DEBUG
else
CPPFLAGS:=$(CPPFLAGS) -O3
endif

LDFLAGS= -pthread
ifdef PROFILE
CPPFLAGS:=$(CPPFLAGS) -pg
LDFLAGS:=$(LDFLAGS) -pg
endif

# Library extension
ifndef LIB_EXT
LIB_EXT:=dll
endif

# Matlab folders
ifndef MATLAB_ROOT
MATLAB_ROOT:="C:/Program Files/MATLAB/R2014b"
endif
ifndef MATLAB_BINDIR
MATLAB_BINDIR:="$(MATLAB_ROOT)/bin/win64"
endif
ifndef MEX_EXT
MEX_EXT:=mexw64
endif
ifndef MATLAB_INCLUDEDIR
MATLAB_INCLUDEDIR:="$(MATLAB_ROOT)/extern/include"
endif

MEX_CPPFLAGS:=$(CPPFLAGS) -I$(MEXDIR) -DMX_COMPAT_32 -DMATLAB_MEX_FILE -I"$(MATLAB_INCLUDEDIR)" -Wall
MEX_LDFLAGS:= $(LDFLAGS) -shared -L"$(MATLAB_BINDIR)" -lstdc++ -lmex -lmx -lmat

MEX_SOURCES = $(shell find $(MEXDIR)/ -type f -name '*_mex.cpp')
MEX_OBJECTS = $(patsubst $(MEXDIR)/%.cpp, $(BUILDDIR)/%.mexo, $(MEX_SOURCES))
MEX = $(patsubst $(MEXDIR)/%_mex.cpp, $(BINDIR)/%.$(MEX_EXT), $(MEX_SOURCES))

CMD_SOURCES = $(shell find $(SRCDIR)/ -type f -name '*_cmd.cpp')
CMD_OBJECTS =  $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(CMD_SOURCES))
CMD =  $(patsubst $(SRCDIR)/%_cmd.cpp, $(BINDIR)/%, $(CMD_SOURCES))

SOURCES := $(shell find $(SRCDIR)/ -type f -name '*.cpp')
SOURCES := $(filter-out $(MEX_SOURCES) $(CMD_SOURCES),$(SOURCES))
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))
LIB := $(BINDIR)/maslib.$(LIB_EXT)

TEST_SOURCES := $(shell find $(TESTDIR)/ -type f -name '*.cpp')
TEST_OBJECTS := $(patsubst $(TESTDIR)/%.cpp, $(BUILDDIR)/%.to, $(TEST_SOURCES))
TEST_CMD := $(patsubst $(TESTDIR)/%.cpp, $(BINDIR)/%, $(TEST_SOURCES))

M_FILES := $(shell find $(MEXDIR)/ -type f -name '*.m')
M_FILES_OUT := $(patsubst $(MEXDIR)/%.m, $(BINDIR)/%.m, $(M_FILES))

HEADERS := $(shell find $(INCLUDEDIR)/ -type f -name '*.h')
# $(notdir $(MEX_SOURCES:.cpp=.$(MEX_EXT))))

MKDIR_CMD=mkdir -p $(@D)

default: all

cleanup:
	rm -rf $(OBJECTS) $(MEX_OBJECTS) $(CMD_OBJECTS) $(TEST_OBJECTS)

clean: cleanup
	rm -rf $(CMD) $(MEX) $(LIB) $(TEST_CMD) $(M_FILES_OUT)

# do not delete intermediates
.SECONDARY:

vars:
	@echo "CMD_SOURCES: $(CMD_SOURCES)"
	@echo "CMD_OBJECTS: $(CMD_OBJECTS)"
	@echo "CMD: $(CMD)"
	@echo "MEX_OBJECTS: $(MEX_OBJECTS)"
	@echo "MEX_CPPFLAGS: $(MEX_CPPFLAGS)"
	@echo "MEX_LDFLAGS: $(MEX_LDFLAGS)"
	@echo "MEX: $(MEX)"
	@echo "TEST_OBJECTS: $(TEST_OBJECTS)"
	@echo "TEST_CMD: $(TEST_CMD)"
	

all: cmd lib mex test

doc:
	@doxygen doc/maslib.doxyfile

mex: $(MEX) $(M_FILES_OUT)

cmd: $(CMD)

lib: $(LIB)

test: $(TEST_CMD)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@$(MKDIR_CMD)
	@echo Compiling $@...
	@$(CC) $(CPPFLAGS) -o $@ -c $<

$(BUILDDIR)/%.mexo: $(MEXDIR)/%.cpp
	@$(MKDIR_CMD)
	@echo Compiling $@...
	@$(CC) $(MEX_CPPFLAGS) -o $@ -c $<
	
$(BUILDDIR)/%.to: $(TESTDIR)/%.cpp
	@$(MKDIR_CMD)
	@echo Compiling $@...
	@$(CC) $(CPPFLAGS) -o $@ -c $<

$(BINDIR)/%.$(MEX_EXT): $(BUILDDIR)/%_mex.mexo $(OBJECTS)
	@$(MKDIR_CMD)
	@echo Assembling $@ ...
	@$(LD) -o $@ $(MEX_LDFLAGS) $(OBJECTS) $<
	
$(BINDIR)/%.m: $(MEXDIR)/%.m
	@$(MKDIR_CMD)
	@echo Copying $@ ...
	@cp $< $@

# Command-line programs
$(BINDIR)/%: $(BUILDDIR)/%.to $(OBJECTS)
	@$(MKDIR_CMD)
	@echo Assembling $@ ...
	@$(LD) -o $@ $(LDFLAGS) $(OBJECTS) $<
		
# Command-line programs
$(BINDIR)/%: $(BUILDDIR)/%_cmd.o $(OBJECTS)
	@$(MKDIR_CMD)
	@echo Assembling $@ ...
	@$(LD) -o $@ $(LDFLAGS) $(OBJECTS) $<
	
$(LIB) : $(OBJECTS)
	@$(MKDIR_CMD)
	@echo Assembling $@ ...
	@ar rcs $@ $(OBJECTS)
	@ranlib $@
	@$(LD) -o $@ $(LDFLAGS) -fPIC -shared $(OBJECTS)
