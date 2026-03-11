

TEST_CMD := ./bin/hyper --test ./src/lexer.c ./bin/test


all:
	./make.lua

%:
	./make.lua "$@"

clean:
	rm -rf tmp
	rm -rf bin

cleanlibs:
	make -C lib/LuaJIT/ clean

test:
	#./bin/hyper --test ./test/main.c ./bin/test.c
	$(TEST_CMD)

gdb:
	gdb --args $(TEST_CMD)


.PHONY: all clean cleanlibs test gdb
