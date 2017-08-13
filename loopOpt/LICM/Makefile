#all: DominatorAnalysis.so DeadCodeElimination.so LoopInvariant.so DominatorsInfo.so FaintAnalysis.so
all: DominatorsInfo.so DeadCodeElimination.so LoopInvariant.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

dataflow.o: dataflow.cpp dataflow.hpp

DominatorAnalysis.o: DominatorAnalysis.h DominatorAnalysis.cpp

FaintAnalysis.o: FaintAnalysis.cpp FaintAnalysis.h

InvariantAnalysis.o: InvariantAnalysis.cpp InvariantAnalysis.h

DeadCodeElimination.o: DeadCodeElimination.cpp

DominatorsInfo.o: DominatorsInfo.hpp DominatorsInfo.cpp 

%.so: %.o dataflow.o FaintAnalysis.o InvariantAnalysis.o DominatorAnalysis.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

cleanest:
	rm -f *.o *~ *.so *.ll ../tests/*.ll

.PHONY: clean all

### Files Registration

## Dominators
PASSFILE = ./DominatorsInfo.so
PASSNAME = -dominators-info
FILENAME=../tests/dom-test0

## Deadcode
# PASSFILE = ./DeadCodeElimination.so
# PASSNAME = -dead-code-elimination
# FILENAME=../tests/dce_test2

## LICM
#PASSFILE = ./LoopInvariant.so
#PASSNAME = -loop-invariant-code-motion
# FILENAME=../tests/loop-invariant
#FILENAME=../tests/nestedloops_test1

### make targets

examplefile:
	llvm-dis $(FILENAME).bc
	mv $(FILENAME).ll .

testfile:
	clang -emit-llvm -S $(FILENAME).c -o $(FILENAME).ll
	opt -mem2reg -loop-simplify -instnamer -S $(FILENAME).ll -o $(FILENAME).ll

testphifile:
	clang -emit-llvm -S $(FILENAME).c -o $(FILENAME).ll
	opt -reg2mem -loop-simplify -instnamer -S $(FILENAME).ll -o $(FILENAME).ll

prepare:
	make cleanest
	make

RunPass:
	make testfile
	opt -load $(PASSFILE) $(PASSNAME) $(FILENAME).ll -S -o out.ll
