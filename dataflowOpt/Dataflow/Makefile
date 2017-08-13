all: liveness.so available.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

dataflow.o: dataflow.cpp dataflow.hpp

zeroanalysis.o: zeroanalysis.cpp zeroanalysis.hpp

AvailableAnalysis.o: AvailableAnalysis.cpp AvailableAnalysis.h

LivenessAnalysis.o: LivenessAnalysis.cpp LivenessAnalysis.h

available-support.o: available-support.cpp available-support.h	

%.so: %.o dataflow.o AvailableAnalysis.o LivenessAnalysis.o available-support.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

cleanest:
	rm -f *.o *~ *.so *.ll

.PHONY: clean all

FILENAME=../tests/$(filename)

ifeq ($(pass),available)
  PASSFILE = ./available.so
  PASSNAME = -available
else ifeq ($(pass),liveness)
  PASSFILE = ./liveness.so
  PASSNAME = -liveness
else
  PASSFILE = ./available.so
  PASSNAME = -available
endif


examplefile:
	llvm-dis $(FILENAME).bc
	mv $(FILENAME).ll .

testfile:
	clang -emit-llvm -S $(FILENAME).c
	opt -mem2reg -instnamer -S $(filename).ll -o $(filename).ll

prepare:
	make cleanest
	make examplefile filename="liveness-test-m2r"
	make examplefile filename="available-test-m2r"
	make testfile filename="test0"
	make testfile filename="test1"
	make testfile filename="test2"
	make testfile filename="test3"
	make testfile filename="test4"
	make testfile filename="test5"
	make

RunPass:
	opt -load $(PASSFILE) $(PASSNAME) $(filename).ll -o out
