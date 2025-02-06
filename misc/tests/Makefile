all : ci

EXAMPLES :=  $(wildcard ../../examples/*/.) $(wildcard ../../examples_v10x/*/.) $(wildcard ../../examples_v20x/*/.) $(wildcard ../../examples_v30x/*/.) $(wildcard ../../examples_x035/*/.)

.PHONY: ci tests all $(EXAMPLES) clean

results :
	mkdir -p results

$(EXAMPLES) : results
	echo $(shell basename $(realpath $(lastword $@)))
	($(MAKE) -C $@ build > results/$(subst .,,$(subst /,_,$@)).txt 2> results/$(subst .,,$(subst /,_,$@)).warning && echo "success" > results/$(subst .,,$(subst /,_,$@)).result) || echo "failure" > results/$(subst .,,$(subst /,_,$@)).result
	echo $(shell basename $(realpath $(lastword $@))).bin > results/$(subst .,,$(subst /,_,$@)).stat
	sha1sum $@/$(shell basename $(realpath $(lastword $@))).bin | cut -d' ' -f 1 >> results/$(subst .,,$(subst /,_,$@)).stat
	wc --bytes $@/$(shell basename $(realpath $(lastword $@))).bin | cut -d' ' -f 1 >> results/$(subst .,,$(subst /,_,$@)).stat

tests : $(EXAMPLES)

ci : install tests

clean :
	rm -rf results

