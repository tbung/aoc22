build/debug/main: main.c
	mkdir -p build/debug
	gcc -g -Wall -o build/debug/main main.c

build/release/main: main.c
	mkdir -p build/release
	gcc -Wall -O3 -o build/release/main main.c

.PHONY: release
release: build/release/main

.PHONY: run
run: main
	./build/main input

.PHONY: clean
clean:
	rm -rf build

.PHONY: test
test: main test_in
	./build/main test_in | diff test_out -
