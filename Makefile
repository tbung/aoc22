YEAR ?= 2022

day%/input:
	mkdir -p $(dir $@)
	curl --cookie cookie.txt https://adventofcode.com/$(YEAR)/day/$(shell echo $@ | sed -E 's/day0?([12]?[0-9]).*/\1/')/input -o $@ -s
