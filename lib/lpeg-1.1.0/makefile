LIBNAME = lpeg
LUADIR = ../LuaJIT/src

COPT = -Os -DNDEBUG
# COPT = -O0 -DLPEG_DEBUG -g

CWARNS = -Wall -Wextra -pedantic \
	-Waggregate-return \
	-Wcast-align \
	-Wcast-qual \
	-Wdisabled-optimization \
	-Wpointer-arith \
	-Wshadow \
	-Wredundant-decls \
	-Wsign-compare \
	-Wundef \
	-Wwrite-strings \
	-Wbad-function-cast \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wnested-externs \
	-Wstrict-prototypes \
	-Wc++-compat \
# -Wunreachable-code \


# CFLAGS = $(CWARNS) $(COPT) -std=c99 -I$(LUADIR) -fPIC -Wno-macro-redefined
CFLAGS = $(CWARNS) $(COPT) -std=c99 -I$(LUADIR) -Wno-macro-redefined -fdata-sections -ffunction-sections
# LFLAGS = -static -no-pie -L$(LUADIR) -lluajit
LFLAGS = rcs
CC = gcc
AR = ar

FILES = lpvm.o lpcap.o lptree.o lpcode.o lpprint.o lpcset.o

# # For Linux
# linux:
# 	$(MAKE) lpeg.so "DLLFLAGS = -shared -fPIC"

# For Mac OS (yuck)
# macosx:
# 	$(MAKE) lpeg.so "DLLFLAGS = -bundle -undefined dynamic_lookup"
# 
# lpeg.so: $(FILES)
# 	env $(CC) $(DLLFLAGS) $(FILES) -o lpeg.so

# test: test.lua re.lua lpeg.so
# 	./test.lua

# test: test.lua re.lua liblpeg.a
# 	./test.lua

# For Linux
linux:
	$(MAKE) liblpeg.a

liblpeg.a: $(FILES)
	env $(AR) $(LFLAGS) liblpeg.a $(FILES)

$(FILES): makefile

clean:
	rm -f $(FILES) lpeg.so liblpeg.a


lpcap.o: lpcap.c lpcap.h lptypes.h
lpcode.o: lpcode.c lptypes.h lpcode.h lptree.h lpvm.h lpcap.h lpcset.h
lpcset.o: lpcset.c lptypes.h lpcset.h lpcode.h lptree.h lpvm.h lpcap.h
lpprint.o: lpprint.c lptypes.h lpprint.h lptree.h lpvm.h lpcap.h lpcode.h
lptree.o: lptree.c lptypes.h lpcap.h lpcode.h lptree.h lpvm.h lpprint.h lpcset.h
lpvm.o: lpvm.c lpcap.h lptypes.h lpvm.h lpprint.h lptree.h

# Since this only compiles to an SO, I can link this with LuaJIT just fine. (heck yeah)

# future me here, what on earth does that previous comment mean? Don't I want a static lib?
