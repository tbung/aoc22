YEAR ?= 2022

# .PHONY: day01 day02 day03 day04 day05 day06 day07 day08 day16
day%: day%/input
	ln -snf ../Makefile.template $@/Makefile
	ln -snf ../launch.json.template $@/launch.json
	cp -n template.c $@/main.c
	cd $@ && bear -- make

day%/input:
	mkdir -p $(dir $@)
	curl --cookie cookie.txt https://adventofcode.com/$(YEAR)/day/$(shell echo $@ | sed -E 's/day0?([12]?[0-9]).*/\1/')/input -o $@ -s

build/stack.o: lib/stack.h
	gcc -xc-header -o build/stack.o lib/stack.h
