YEAR ?= 2022

day%: day%/input
	ln -s ../Makefile.template $@/Makefile
	cp -n template.c $@/main.c
	cd $@ && bear -- make

day%/input:
	mkdir -p $(dir $@)
	curl --cookie cookie.txt https://adventofcode.com/$(YEAR)/day/$(shell echo $@ | sed -E 's/day0?([12]?[0-9]).*/\1/')/input -o $@ -s
