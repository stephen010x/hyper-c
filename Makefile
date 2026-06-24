
SRCDIR := src
OUTDIR := build
TMPDIR := $(OUTDIR)/tmp
TARGET := $(OUTDIR)/hyper
# CC := gcc
CC := musl-gcc

SRCS := main.c lexer.c parser.c


CWARNS := -Wall -Wextra \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wstrict-prototypes \
	-Wunreachable-code \
	-Wcomplain-wrong-lang \
	-Wcast-align \
	-Wcast-qual \
	-Wdisabled-optimization \
	-Wpointer-arith \
	-Wshadow=local \
	-Wredundant-decls \
	-Wsign-compare \
	-Wundef \
	-Wbad-function-cast \
	-Wno-multichar \
	-Wno-dangling-else
	# -Winline
	# -fmax-errors=10
	# -Wnested-externs
	# -Wwrite-strings
	# -Wno-macro-redefined
	# -pedantic


# optimizations to speed up code for -Os
# note, testing indicates that many of these may be rejected by -Os due to size assertions
# COPTIM := -Os -g0
COPTIM := -Os \
	-flto \
	-fno-pic \
	-fgcse-after-reload \
	-floop-interchange \
	-fpredictive-commoning \
	-ftree-loop-distribution \
	-ftree-partial-pre \
	-fvect-cost-model=dynamic \
	-freorder-blocks-algorithm=stc \
	-fno-semantic-interposition \
	-fipa-pta \
	-fmodulo-sched \
	-fmodulo-sched-allow-regmoves \
	-fgcse-sm \
	-fgcse-las \
	-finline-stringops \
	-fno-stack-protector \
	-fno-zero-initialized-in-bss \
	-fstack-check=no \
	-fno-stack-clash-protection \
	-fvisibility=hidden \
	-fno-exceptions \
	-fcf-protection=none \
	-fomit-frame-pointer \
	-fno-asynchronous-unwind-tables \
	-fno-unwind-tables \
	-fno-ident \
	-fdata-sections \
	-ffunction-sections \
	-fno-fat-lto-objects \
	-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
	# -fprefetch-loop-arrays


# LOPTIM := -s
LOPTIM := -s \
	-flto \
	-no-pie \
	-static-libgcc \
	-Wl,-O2 \
	-Wl,--gc-sections \
	-Wl,--print-gc-sections \
	-Wl,--as-needed \
	-Wl,--no-export-dynamic \
	-Wl,-z,norelro \
	-Wl,-z,max-page-size=4096 \
	-Wl,-z,common-page-size=4096 \
	-Wl,--build-id=none \
	-Wl,-z,noseparate-code \
	-Wl,--relax \
	-Wl,-s
	# -Wl,--hash-style=gnu \
	# -Wl,--omagic
	# -Wl,-z,stack-size=65536


CDEBUG := -g3 -Og \
	-ftrivial-auto-var-init=pattern \
	-fsanitize=address \
	-fsanitize=undefined \
	-fsanitize=leak \
	-fstack-protector-all \
	-fanalyzer-verbosity=0 \
	-fstack-usage
	# -fanalyzer
	# -fsanitize=thread

LDEBUG := \
	-fsanitize=address \
	-fsanitize=undefined \
	-fsanitize=leak \
	-static-libasan \
	-static-libubsan


CCLIBS := -Ilib/toolbox/inc/
LDLIBS :=

CCFLAGS := -std=gnu17 $(CWARNS) -ftrack-macro-expansion
LDFLAGS :=
# LDFLAGS := -static


ifdef DEBUG	
	CC := gcc
	CCFLAGS += $(CDEBUG) -D__DEBUG__ -D__debug__
	LDFLAGS += $(LDEBUG)
else
	CCFLAGS += $(COPTIM)
	LDFLAGS += $(LOPTIM)
endif




SRCS := $(SRCS:%=$(SRCDIR)/%)
OBJS := $(SRCS:%.c=$(TMPDIR)/%.o)
DEPS := $(OBJS:.o=.d)


# strip --remove-section=.comment
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TMPDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -MMD -MP -c $< $(CCLIBS) -o $@

-include $(DEPS)

$(OBJS): Makefile




fast:
	make FAST=1

debug:
	make DEBUG=1

clean:
	rm -rf $(OUTDIR)
	rm -rf $(TMPDIR)

clean-purge:
	:

test:
	# $(TARGET) --test ./src/lexer.c ./bin/test
	./$(TARGET) ./src/lexer.c
	./$(TARGET) ./test/benchmark.c
	./$(TARGET) ./test/main.c
	./$(TARGET) ./test/test.c

# gdb:
# 	gdb --args $(TEST_CMD)


# TODO: look into -fprofile-generate and Profile Guided Optimizations
# https://gist.github.com/daniel-j-h/c4b109bff0b717fc9b24
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
# Also look into BOLT like perf2bolt
# makeprofile:
# 	sudo perf record -g -F 20000 luajit make.lua
# 	sudo perf report
# 
# benchmark:
# 	nm -S --size-sort executable
# 	objdump -h $(BINDIR)/$(TARGET)
# 	readelf -S build/hyperc
# 	readelf -SW $(BINDIR)/$(TARGET)
# 	size $(BINDIR)/$(TARGET)
# 	size -A $(BINDIR)/$(TARGET)
# 	hyperfine --warmup 5 --min-runs 30 $(OUTDIR)/$(TARGET)
# 	perf stat $(OUTDIR)/$(TARGET) typical_input


# building musl
# run installer script with these, 
# ./configure --disable-shared --enable-gcc-wrapper

# Nevermind. It is on apt
# sudo apt install musl musl-dev musl-tools


.PHONY: all fast debug clean clean-purge test gdb 
.PHONY: makeprofile benchmark
