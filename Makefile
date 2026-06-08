

TEST_CMD := ./bin/hyperc --test ./src/lexer.c ./bin/test


all:
	./make.lua

%:
	./make.lua "$@"

clean:
	rm -rf tmp
	rm -rf bin

cleanlibs:
	make -C lib/LuaJIT/ clean
	make -C lib/lpeg-1.1.0/ clean

test:
	@# ./bin/hyper --test ./test/main.c ./bin/test.c
	$(TEST_CMD)

gdb:
	gdb --args $(TEST_CMD)


makeprofile:
	# luajit -jp=f5rm1i1,makeprofile.log make.lua
	# cat makeprofile.log
	sudo perf record -g -F 20000 luajit make.lua
	sudo perf report


pull-submodules:
	git submodule update --init --recursive


.PHONY: all clean cleanlibs test gdb pull-submodules makeprofile
