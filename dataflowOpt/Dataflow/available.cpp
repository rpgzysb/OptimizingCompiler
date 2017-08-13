// 15-745 S16 Assignment 2: available.cpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "AvailableAnalysis.h"
#include "available-support.h"

using namespace llvm;
using namespace std;

namespace {
  // the available expressions information pass
  class AvailableExpressions : public FunctionPass {

  public:
    static char ID;

    AvailableExpressions() : FunctionPass(ID) { }

    virtual bool runOnFunction(Function& F) {
      // create an instance of available analysis
      AvailableAnalysis aa{F};
      // iterate through all basic blocks
      for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
        BasicBlock* block = FI;
        // name of current basic block
        std::string bb_name{block->getName().str()};
        // get the states of all instructions within this basic block
        std::vector<AvailableAnalysis::FlowSet> ins_states{aa.getInstructionStates(bb_name)};
        
        int idx = 0;
        for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
          Instruction *I = i;

          // get the state of current instruction
          AvailableAnalysis::FlowSet curr_flow{ins_states[idx]};
          // print out instruction and available expressions
          outs() << "\t\t\t\t";
          I->print(outs());
          outs() << "\n";

          outs() << "{";
          for (auto expvp : curr_flow) {
            if (expvp.second) {
              outs() << expvp.first.toString() << ", ";
            }
          }
          outs() << "}\n";

          idx++;
        }
      }
      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

  private:
  };

  char AvailableExpressions::ID = 0;
  RegisterPass<AvailableExpressions> X("available",
    "15745 Available Expressions");
}
