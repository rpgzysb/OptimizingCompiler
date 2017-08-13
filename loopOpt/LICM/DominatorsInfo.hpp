// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef _DOMINATORS_INFO_
#define _DOMINATORS_INFO_

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "DominatorAnalysis.h"

using namespace llvm;

namespace {

  class DominatorsInfo : public FunctionPass 
  {
  public:
    static char ID;

    DominatorsInfo() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function& F)
    {
      
      DominatorAnalysis da{F};
      for (auto bb = F.begin(); bb != F.end(); ++bb) {
        std::string bb_name{bb->getName().str()};

        DominatorAnalysis::FlowSet dominators{da.getFlowAfter(bb_name)};
        for (auto d : dominators) {
         outs() << d << " dom " << bb_name << "\n";
        }
      }
      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const
    {
      AU.setPreservesAll();
    }
  };

  char DominatorsInfo::ID = 0;
  RegisterPass<DominatorsInfo> X("dominators-info", "15745 DominatorsInfo");

}

#endif