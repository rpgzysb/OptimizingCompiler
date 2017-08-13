// 15-745 S16 Assignment 2: liveness.cpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "LivenessAnalysis.h"

using namespace llvm;

namespace {
  // The liveness information pass
  class Liveness : public FunctionPass {
  public:
    static char ID;

    Liveness() : FunctionPass(ID) { }

    virtual bool runOnFunction(Function& F) {
      // instance of a liveness analysis class
      LivenessAnalysis la{F};
      for (auto bb = F.begin(); bb != F.end(); ++bb) {
        // name of current basic block
        std::string bb_name{bb->getName().str()};
        // get the states of all instructions within this basic block
        std::vector<LivenessAnalysis::FlowSet> 
        ins_states{la.getInstructionStates(bb_name)};
        // iterate the states from backward
        int idx = ins_states.size() - 1;
        // begin iterate through basic blocks
        for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
          // we only care about these instructions
          if (isa<Instruction>(ins) || isa<Argument>(ins)) {
            // print out states
            outs() << "{";
            LivenessAnalysis::FlowSet curr_flow{ins_states[idx]};
            for (auto kvp : curr_flow) {
              if (kvp.second) {
                outs() << kvp.first << ", ";
              }
            }
            outs() << "}\n";
          }
          // write out all instructions
          outs() << "\t\t\t\t";
          ins->print(outs());
          outs() << "\n";
          idx--;
        }
      }
      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

  private:
  };

  char Liveness::ID = 0;
  RegisterPass<Liveness> X("liveness", "15745 Liveness");
}
