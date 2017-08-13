// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"

#include "FaintAnalysis.h"

using namespace llvm;

namespace {

  class DeadCodeElimination : public FunctionPass {
  public:
    static char ID;

    DeadCodeElimination() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function& F) {
      FaintAnalysis fa{F};
      std::vector<Instruction*> deadInsts;
      bool dceFlag = false; // false if no dce is actually performed

      for (auto bb = F.begin(); bb != F.end(); ++bb) {

        std::string bb_name{bb->getName().str()};
        std::vector<FaintAnalysis::FlowSet>
          ins_states{fa.getInstructionStates(bb_name)};

        int idx = ins_states.size() - 1;
        //deadInsts.clear();

        for (BasicBlock::iterator ins = bb->begin(); ins != bb->end(); ++ins) {
          
          FaintAnalysis::FlowSet curr_flow{ins_states[idx]};

          outs() << "\t\t";
          ins->print(outs());
          outs() << "\n";

          if (isa<Instruction>(ins) || isa<Argument>(ins)) {
            std::string def_name{ins->getName().str()};
          
            if (!isLive(ins) && curr_flow[def_name]) {
              //ins->eraseFromParent();
              Instruction * inst = &*ins;
              deadInsts.push_back(inst);
              outs() << "dead\n";
              dceFlag = true;
            }
            else {
              outs() << "live\n";
            }
          }
          idx--;
        }

      }

      if (dceFlag) {
        outs() << "\n Deleting: \n $$$$$";
        //for (Instruction* inst : deadInsts) {
        std::vector<Instruction*>::reverse_iterator rit = deadInsts.rbegin();
        for (; rit != deadInsts.rend(); ++rit) {
          Instruction* inst = *rit;
          
          outs() << "\t\t"; 
          inst->print(outs());
          outs() << "\n";

          //inst->replaceAllUsesWith(Constant::getNullValue(PointerType::getUnqual(Type::Int32Ty)));
          //inst->replaceAllUsesWith(Constant::getNullValue(PointerType::getUnqual(Type::getInt32Ty(getGlobalContext())->getPointerTo())));
          //inst->replaceAllUsesWith(Constant::getNullValue(PointerType::getUnqual(inst->getType())));
          inst->replaceAllUsesWith(UndefValue::get(inst->getType()));
          //inst->removeFromParent();
          inst->eraseFromParent();
          
        }
        outs() << "\n $$$$$\n";
      }
      return dceFlag;
    }

    bool hasPhi(Instruction* I) {
      if (isa<PHINode>(I)) {
        return true;
      }
      //for ()
      return false;
    }

    bool isLive(Instruction* I) {
      return isa<TerminatorInst>(I) || isa<DbgInfoIntrinsic>(I) ||
        isa<LandingPadInst>(I) || I->mayHaveSideEffects();
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

  private:
  };

  char DeadCodeElimination::ID = 0;
  RegisterPass<DeadCodeElimination> X{"dead-code-elimination", "15745 DeadCodeElimination"};

}