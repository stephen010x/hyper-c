
TARGET := hyperc
SRCDIR := src
OUTDIR := build
TMPDIR := $(OUTDIR)/tmp
# CC := gcc
CC := musl-gcc

SRCS := main.c lexer.c parser.c


CWARNS := -Wall -Wextra -pedantic \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wunreachable-code \
	-Wcomplain-wrong-lang \
	-Wcast-align \
	-Wcast-qual \
	-Wdisabled-optimization \
	-Wpointer-arith \
	-Wshadow \
	-Wredundant-decls \
	-Wsign-compare \
	-Wundef \
	-Wbad-function-cast \
	-Wmissing-declarations \
	-Wno-multichar
	# -Wnested-externs \
	# -Wwrite-strings \
	# -Wno-macro-redefined \


# optimizations to speed up code for -Os
# note, testing indicates that many of these may be rejected by -Os due to size assertions
COPTIM := -Os -g0 \
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
	# -fprefetch-loop-arrays \


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
	-Wl,--relax
	# -Wl,-z,stack-size=65536 \


CDEBUG := -g3 -O0 \
	-ftrivial-auto-var-init=pattern \
	-fsanitize=address \
	-fsanitize=undefined \
	-fsanitize=leak \
	-fstack-protector-all \
	-fanalyzer \
	-fstack-usage
	# -fsanitize=thread

LDEBUG := \
	-fsanitize=address \
	-fsanitize=undefined \
	-fsanitize=leak


CCLIBS := -Ilib/toolbox/inc/
LDLIBS :=

CCFLAGS := -std=gnu17
LDFLAGS :=
# LDFLAGS := -static


ifdef DEBUG
	CCFLAGS += $(CDEBUG) $(CWARNS) -D__DEBUG__ -D__debug__
	LDFLAGS += $(LDEBUG)
else
	CCFLAGS += $(COPTIM)
	LDFLAGS += $(LOPTIM)
endif




SRCS := $(SRCS:%=$(SRCDIR)/%)
OBJS := $(SRCS:%.c=$(TMPDIR)/%.o)
DEPS := $(OBJS:.o=.d)


$(OUTDIR)/$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TMPDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -MMD -MP -c $< $(CCLIBS) -o $@

-include $(DEPS)



clean:
	rm -rf $(OUTDIR)
	rm -rf $(TMPDIR)

clean-purge:
	:

# test:
# 	./bin/hyperc --test ./src/lexer.c ./bin/test

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
# 	objdump -h $(BINDIR)/$(TARGET)
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


.PHONY: all clean clean-purge test gdb 
.PHONY: makeprofile benchmark
