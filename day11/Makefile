.PHONY: run
run: main
	./main input

main: main.c
	gcc -g -Wall -o main main.c

.PHONY: clean
clean:
	rm -rf main

.PHONY: test
test: main test_in
	./main test_in | diff test_out -
