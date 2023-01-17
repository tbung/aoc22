main: main.c
	mkdir -p build
	gcc -g -Wall -o build/main main.c

.PHONY: run
run: main
	./build/main input

.PHONY: clean
clean:
	rm -rf build

.PHONY: test
test: main test_in
	./build/main test_in | diff test_out -
