CLANG := clang-11
OPT := opt-11.0.1
PYTHON3 := python3
CFLAGS ?= -O3 -fno-vectorize -fno-slp-vectorize -fno-unroll-loops -I../

BENCHNAME ?= $(shell basename `pwd`)

LLVM_DFG_PLUGIN ?= ../../../build/libPassModule2DFG.so

$(BENCHNAME): $(BENCHNAME).ll
	$(OPT) -load $(LLVM_DFG_PLUGIN) -module2dfg -S $(BENCHNAME).ll -disable-output --enable-new-pm=0
	mv opgraph.txt $(BENCHNAME)_DFG.txt
	rm $(BENCHNAME).ll
	python3 ../compat.py $(BENCHNAME)_DFG.txt $(BENCHNAME)_compat.txt

$(BENCHNAME).ll: 
	$(CLANG) $(CFLAGS) -emit-llvm -S $(BENCHNAME).c -o $(BENCHNAME).ll

compile: $(BENCHNAME)

draw: $(BENCHNAME)
	$(PYTHON3) ../../../tool/drawGraph.py $(BENCHNAME)_DFG.txt
	rm $(BENCHNAME)_DFG.txt.dot
	mv $(BENCHNAME)_DFG.txt.dot.pdf $(BENCHNAME).pdf

clean:
	rm -f $(BENCHNAME).ll $(BENCHNAME)_DFG.txt $(BENCHNAME)_compat.txt $(BENCHNAME).pdf
