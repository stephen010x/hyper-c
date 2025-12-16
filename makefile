

TEST_CMD := ./bin/hyper --test ./src/lexer.c ./bin/test.c


all:
	./make.lua

%:
	./make.lua "$@"

clean:
	rm -rf tmp
	rm -rf bin

test:
	#./bin/hyper --test ./test/main.c ./bin/test.c
	$(TEST_CMD)

gdb:
	gdb --args $(TEST_CMD)


.PHONY: all clean test gdb
