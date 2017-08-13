all: FunctionInfo.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

%.so: %.o
	$(CXX) -dylib -shared $^ -o $@
clean:
	rm -f *.o *~ *.so
cleanest:
	rm -f *.o *~ *.so
	rm -f *.bc *.ll out*