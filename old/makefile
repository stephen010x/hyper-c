# gcc options
# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# ld options
# https://sourceware.org/binutils/docs/ld/Options.html


TARGET := bitwin
INCLUDEE := bitwin.h

TMPDIR := tmp
SRCDIR := src
INCDIR := include
BINDIR := bin
# this makefile expects libraries to use the same directory structure and names as specified above
LIBDIR := lib


WINDOWS_LIBS :=
LINUX_LIBS   :=
# TODO: consider renaming "wasm" to "web"
WASM_LIBS    :=


CC := gcc
# change fvisibility for when you turn this into a library
#CFLAGS := -fvisibility=hidden -DENABLE_BITWIN_TEST_C -std=c23
# NOTE: -std=c23 is strict c23 compliance, whereas -std=gnu23 includes gnu specific features
# Here are the extensions
# https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html
CFLAGS := -DENABLE_BITWIN_TEST_C -std=gnu23 -fvisibility=internal
# -static
#LFLAGS := -Wl,-nmagic -Wpedantic
LFLAGS := -no-pie -lutils
BFLAGS := -Wall -Wextra
#LIBS   :=

DB := gdb
DBFLAGS :=

AR := ar
ARFLAGS := rcs

PP := m4


FAST_CFLAGS := -D__DEBUG__ -D__debug__
FAST_LFLAGS := -lGL
FAST_BFLAGS := -g -O0 

DEBUG_CFLAGS := -D__DEBUG__ -D__debug__
DEBUG_LFLAGS := -lGL
DEBUG_BFLAGS := -g -Og

RELEASE_CFLAGS :=
# place each function and data into its own section so that they can get removed by --gc-sections if unused
# -Wl,-Bsymbolic
# remove or change --exclude-libs,ALL for when you turn this into a library
RELEASE_LFLAGS := -s -lGL -ffunction-sections -fdata-sections -Wl,--gc-sections,--exclude-libs,ALL
RELEASE_BFLAGS := -Os -g0


STATIC_CFLAGS := -D__STATIC_LIB__
STATIC_LFLAGS := 
STATIC_BFLAGS := 

DYNAMIC_CFLAGS := -D__DYNAMIC_LIB__ -fpic
DYNAMIC_LFLAGS := 
DYNAMIC_BFLAGS := 


WINDOWS_CFLAGS := -D__WIN32__
WINDOWS_LFLAGS :=
WINDOWS_BFLAGS :=

LINUX_CFLAGS := -D__LINUX__
LINUX_LFLAGS := -lX11 -lrt
# dammit no!!!
#LINUX_BFLAGS := -mx32 -m32
LINUX_BFLAGS := -m32
#LINUX_LIBS   :=

WASM_CFLAGS := -D__WEBASM__
WASM_LFLAGS :=
WASM_BFLAGS :=



DEFAULT := static_release
WHITELIST := all fast release debug static dynamic 
WHITELIST := $(WHITELIST) static_fast static_debug static_release
WHITELIST := $(WHITELIST) dynamic_fast dynamic_debug dynamic_release


# ============================================
# ============================================


# so the first word is the goal, and subsiquent words are modifiers
# the goal is basically optimization target, but subsiquent goal can be static
GOAL := $(firstword $(MAKECMDGOALS))
GOAL := $(if $(GOAL),$(GOAL),$(DEFAULT))
# should grab all sources relative to makefile.
# colon equal (:=) removed, so as to grab all generated c files as well
SRCS = $(shell find $(SRCDIR) -name "*.c")
INCS = $(shell find $(INCDIR) -name "*.h")
ifneq ($(filter $(GOAL),$(WHITELIST)),)
	OBJS = $(SRCS:%.c=$(TMPDIR)/$(GOAL)/%.o)
	DEPS = $(SRCS:%.c=$(TMPDIR)/$(GOAL)/%.d)
endif

#BINGOAL :=  $(subst dynamic_,,$(subst static_,,$(GOAL)))
#BINTARG := $(BINDIR)/$(BINGOAL)/$(TARGET)
BINTARG := $(BINDIR)/$(TARGET)
LIBBINTARG := $(BINDIR)/lib$(TARGET)
INCTARG := $(BINDIR)/$(INCLUDEE)

LIBDIRS := $(shell ls -d $(LIBDIR)/*/)
LIBINCS := $(foreach path,$(LIBDIRS),-I$(path)$(INCDIR)/)
LIBBINS := $(foreach path,$(LIBDIRS),-L$(path)$(BINDIR)/)
CFLAGS  := $(CFLAGS) $(LIBINCS)
LFLAGS  := $(LFLAGS) $(LIBBINS)


ifeq ($(OS),Windows_NT) 
    $(error Winbows not yet implemented)
else
    OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
    ifeq ($(OS),Linux)
    	LINUX := LINUX
    	#BLFLAGS += -target *-*-linux-gnu
    	#MAKEOSFILE := make/linux.mk
    else
		$(error Incompatable operating system)
    endif
endif



#==========================================
#==========================================


all: $(DEFAULT)
fast: _fast $(BINTARG)
debug: _debug $(BINTARG)
release: _release $(BINTARG)

static: static_$(DEFAULT)
static_fast: _static _fast $(LIBBINTARG).a
static_debug: _static _debug $(LIBBINTARG).a
static_release: _static _release $(LIBBINTARG).a


-include $(DEPS)


# -sGL_DEBUG=1
_fast:
	$(eval CFLAGS += $(FAST_CFLAGS))
	$(eval LFLAGS += $(FAST_LFLAGS))
	$(eval BFLAGS += $(FAST_BFLAGS))

# -sGL_DEBUG=1
_debug:
	$(eval CFLAGS += $(DEBUG_CFLAGS))
	$(eval LFLAGS += $(DEBUG_LFLAGS))
	$(eval BFLAGS += $(DEBUG_BFLAGS))

# -Oz  ## smaller than -Os, but also slower
#  -sMODULARIZE
_release:
	$(eval CFLAGS += $(RELEASE_CFLAGS))
	$(eval LFLAGS += $(RELEASE_LFLAGS))
	$(eval BFLAGS += $(RELEASE_BFLAGS))	

_static:
	$(eval CFLAGS += $(STATIC_CFLAGS))
	$(eval LFLAGS += $(STATIC_LFLAGS))
	$(eval BFLAGS += $(STATIC_BFLAGS))

_dynamic:
	$(eval CFLAGS += $(DYNAMIC_CFLAGS))
	$(eval LFLAGS += $(DYNAMIC_LFLAGS))
	$(eval BFLAGS += $(DYNAMIC_BFLAGS))



#=========================================#
#      LINUX                              #
#=========================================#
ifdef LINUX


CFLAGS += $(LINUX_CFLAGS)
LFLAGS += $(LINUX_LFLAGS)
BFLAGS += $(LINUX_BFLAGS)


dynamic: dynamic_$(DEFAULT)
dynamic_fast: _dynamic _fast $(LIBBINTARG).so
dynamic_debug: _dynamic _debug $(LIBBINTARG).so
dynamic_release: _dynamic _release $(LIBBINTARG).so


$(BINTARG): $(OBJS)
	mkdir -p $(dir $@)
	#$(CC) $(BFLAGS) -o $@ -o $(basename $@) $^ -L$(LIBDIR) $(LFLAGS)
	$(CC) $(BFLAGS) -o $@ $^ -L$(LIBDIR) $(LFLAGS)

$(LIBBINTARG).a: $(OBJS)
	mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

$(LIBBINTARG).so: $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $(BFLAGS) -o $@ $^ -L$(LIBDIR) $(LFLAGS)

# Lets just implement header compilation and consolidation later
#$(INCTARG): $(HEADERS)
#	$(PP) $(INCTARG)
#	$(CC) -I$(INCDIR) $(BFLAGS) $(CFLAGS) -E -P -fpreprocessed include/bitwin.h -o bin/bitwin.pp.h
#	# remember to create a compiled version of the header

$(TMPDIR)/$(GOAL)/%.o: %.c $(TMPDIR)/$(GOAL)/%.d
	mkdir -p $(dir $@)
	$(CC) -I$(INCDIR)  $(BFLAGS) $(CFLAGS) -c $< -o $@

$(TMPDIR)/$(GOAL)/%.d: %.c
	mkdir -p $(dir $@)
	$(CC) -I$(INCDIR) $(BFLAGS) $(CFLAGS) -MM -MT $(patsubst %.d,%.o,$@) -MF $@ $<



clean:
	rm -rf $(TMPDIR)
	rm -rf $(BINDIR)


run:
	./$(BINTARG)


gdb:
	gdb -x script.gdb ./$(BINTARG)


analyze:
	readelf -a $(BINTARG)


endif
#==========================================
#==========================================




# Example rule behavior
#
# rule: dependancies
#	:
#
# I don't remember what this does, but it is useful, because rules without it behave differently



# TODO: just have this run on every c and header file, and dump into special directory in tmp directory
preprocess:
	#gcc -DENABLE_BITWIN_TEST_C -D__DEBUG__ -D__LINUX__ -E src/linux/bitwin.c -o bin/bitwin.pp.c
	#gcc -I$(INCDIR) -DENABLE_BITWIN_TEST_C -D__DEBUG__ -D__LINUX__ -E -dD include/bitwin.h -o bin/bitwin.pp.h
	#gcc -I$(INCDIR) -DENABLE_BITWIN_TEST_C -D__DEBUG__ -D__LINUX__ -E -P -fpreprocessed include/bitwin.h -o bin/bitwin.pp.h
	#gcc -I$(INCDIR) -DENABLE_BITWIN_TEST_C -D__DEBUG__ -D__LINUX__ -E -P include/bitwin.h -o bin/bitwin.pp.h
	$(CC) -I$(INCDIR)  $(BFLAGS) $(CFLAGS) -E -P src/linux/bitwin.c -o bin/bitwin.pp.c




#.PHONY: all fast debug release static dynamic preprocess
.PHONY: all fast debug release static dynamic preprocess
.PHONY: static_fast static_debug static_release dynamic_fast dynamic_debug dynamic_release
.PHONY: clean test shaders run gdb analyze
.PHONY: _build _fast _debug _release _dynamic _static _optimize
