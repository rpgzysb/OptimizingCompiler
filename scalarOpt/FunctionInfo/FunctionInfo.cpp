// 15-745 S16 Assignment 1: FunctionInfo.cpp
// Group:
// Xian Zhang (xianz)
// Yuzhong Zhang (yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>

using namespace llvm;

namespace {
  class FunctionInfo : public FunctionPass {
  public:
    static char ID;
    FunctionInfo() : FunctionPass(ID) { }
    ~FunctionInfo() { }

    // We don't modify the program, so we preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    // Do some initialization
    bool doInitialization(Module &M) override {
      outs() << "Name,\tArgs,\tCalls,\tBlocks,\tInsns\n";

      return false;
    }

    // Print output for each function
    bool runOnFunction(Function &F) override {
      //outs() << "name" << ",\t" << "args" << ",\t" << "calls" << ",\t" << "bbs" << ",\t" << "insts" << "\n";
      int callsiteCnt = 0, instCnt = 0;
      // name
      outs() << F.getName() << ",\t";

      // args
      if (F.isVarArg()) {
        outs() << "*" << ",\t";
      } else {
        outs() << F.arg_size() << ",\t";
      }

      // iterate basic blocks
      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        // inst count
        instCnt += b->size();
      }

      // direct call sites && bbs && insts 
      outs() << F.getNumUses() << ",\t" << F.size() << ",\t" << instCnt << "\n";
      return false;
    }
  };
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char FunctionInfo::ID = 0;
static RegisterPass<FunctionInfo> X("function-info", "15745: Function Information", false, false);
