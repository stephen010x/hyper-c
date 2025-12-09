all:
	./make.lua

%:
	./make.lua "$@"

clean:
	rm -rf tmp
	rm -rf bin

test:
	./bin/hyper ./test/main.c -o ./bin/test


.PHONY: all clean test
