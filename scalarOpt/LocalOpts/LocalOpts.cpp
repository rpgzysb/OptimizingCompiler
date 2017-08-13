// 15-745 S16 Assignment 1: FunctionInfo.cpp
// Group:
// Xian Zhang (xianz)
// Yuzhong Zhang (yuzhongz)
////////////////////////////////////////////////////////////////////////////////
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <iostream>

#define DEBUG 0
#define AL 1
#define CF 1
#define ST 1

using namespace llvm;

namespace {
  class LocalOpts : public FunctionPass {
  public:
    static char ID;
    LocalOpts() : FunctionPass(ID) {}
    ~LocalOpts() {}

    int algebraicOptNum, constfoldOptNum, strengthOptNum;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    // Do some initialization
    bool doInitialization(Module &M) override {
      outs() << "LocalOpts" << '\n';
      algebraicOptNum = 0;
      constfoldOptNum = 0;
      strengthOptNum = 0;

      return false;
    }

    int getShift(long x) {
      // http://www.exploringbinary.com/
      // ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
      // if not power of two, return
      if (x <= 0 || (x & (~x + 1)) != x) {
        return -1;
      }
      // if power of two, get the number of shifts necessary
      int i = 0;
      while (x > 1) {
        x >>= 1;
        ++i;
      }
      return i;
    }

    void deleteDeadInsts(std::vector<Instruction*> insts) {
      for (std::vector<Instruction*>::iterator i = insts.begin(); 
        i != insts.end(); ++i)
        (*i)->eraseFromParent();
    }

    void constfold(Function &F) {
      std::vector<Instruction*> deadInsts;

      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        // iterate insts
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); 
          i != ie; ++i) {
          if (i->getNumOperands() == 2) {
            long constVal1, constVal2;
            Value* opd1 = i->getOperand(0);
            Value* opd2 = i->getOperand(1);

            // get int64 signed for ReplaceInstWithValue
            if (isa<ConstantInt>(opd1)) {
              constVal1 = dyn_cast<ConstantInt>(opd1)->getSExtValue();
            }
            if (isa<ConstantInt>(opd2)) {
              constVal2 = dyn_cast<ConstantInt>(opd2)->getSExtValue();
            }

            if (isa<ConstantInt>(opd1) && isa<ConstantInt>(opd2)) {
              // constant folding
              bool flag = true;
              switch (i->getOpcode()) {
                case Instruction::Add:
                  //ReplaceInstWithValue(i->getParent()->getInstList(), i, 
                  //  ConstantInt::getSigned(i->getType(), constVal1 + constVal2));
                  i->replaceAllUsesWith(ConstantInt::getSigned(i->getType(), 
                    constVal1 + constVal2));
                  break;
                case Instruction::Sub:
                  //ReplaceInstWithValue(i->getParent()->getInstList(), i, 
                  //  ConstantInt::getSigned(i->getType(), constVal1 - constVal2));
                  i->replaceAllUsesWith(ConstantInt::getSigned(i->getType(), 
                    constVal1 - constVal2));
                  break;
                case Instruction::Mul:
                  //ReplaceInstWithValue(i->getParent()->getInstList(), i, 
                  //  ConstantInt::getSigned(i->getType(), constVal1 * constVal2));
                  i->replaceAllUsesWith(ConstantInt::getSigned(i->getType(), 
                    constVal1 * constVal2));
                  break;
                case Instruction::SDiv:
                  //ReplaceInstWithValue(i->getParent()->getInstList(), i, 
                  //  ConstantInt::getSigned(i->getType(), constVal1 / constVal2));
                  if (constVal2 != 0) {
                    i->replaceAllUsesWith(ConstantInt::getSigned(i->getType(), 
                    constVal1 / constVal2));
                  } else {
                    flag = false;
                  }
                  break;
                default:
                  flag = false;
                  break;
              }
              if (flag) {
                deadInsts.push_back(i); // store to delete later
                if (DEBUG) {
                  outs() << "[CF] ";
                  i->print(errs());
                  outs() << "\n";
                }
                ++constfoldOptNum;
              }
            }
          }
        }
        deleteDeadInsts(deadInsts);
      }
    }

    void algebraic(Function &F) {
      std::vector<Instruction*> deadInsts;
      
      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); 
          i != ie; ++i) {
          if (i->getNumOperands() == 2) {

            long constVal1, constVal2;
            Value* opd1 = i->getOperand(0);
            Value* opd2 = i->getOperand(1);

            // get int64 signed for ReplaceInstWithValue
            if (isa<ConstantInt>(opd1)) {
              constVal1 = dyn_cast<ConstantInt>(opd1)->getSExtValue();
            }
            if (isa<ConstantInt>(opd2)) {
              constVal2 = dyn_cast<ConstantInt>(opd2)->getSExtValue();
            }
            bool algebraicFlag = true;
            switch (i->getOpcode()) { 
              case Instruction::Add:
                if (isa<ConstantInt>(opd1) && constVal1 == 0) {
                  // 0 + x
                  i->replaceAllUsesWith(opd2);
                } else if (isa<ConstantInt>(opd2) && constVal2 == 0) {
                  // x + 0
                  i->replaceAllUsesWith(opd1);
                } else {
                  algebraicFlag = false;
                }
                break;
              case Instruction::Sub:
                if (isa<ConstantInt>(opd2) && constVal2 == 0) {
                  // x - 0
                  i->replaceAllUsesWith(opd1);
                } else if (opd1 == opd2) {
                  // x - x
                  i->replaceAllUsesWith(ConstantInt::getSigned(i->getType(), 
                    0));
                } else {
                  algebraicFlag = false;
                }
                break;
              case Instruction::Mul:
                if (isa<ConstantInt>(opd1) && constVal1 == 1) {
                  // 1 * x
                  i->replaceAllUsesWith(opd2);
                } else if (isa<ConstantInt>(opd2) && constVal2 == 1) { 
                  // x * 1
                  i->replaceAllUsesWith(opd1);
                } else {
                  algebraicFlag = false;
                }
                break;
              case Instruction::SDiv:
                if (isa<ConstantInt>(opd2) && constVal2 == 1) { 
                  // x / 1
                  i->replaceAllUsesWith(opd1);
                } else if (opd1 == opd2) {                  
                  // x / x
                  i->replaceAllUsesWith(ConstantInt::getSigned(i->getType(), 
                    1));
                } else {
                  algebraicFlag = false;
                }
                break;
              default:
                algebraicFlag = false;
                break;
            }
            if (algebraicFlag) {
              if (DEBUG) {
                outs() << "[AL] ";
                i->print(errs());
                outs() << "\n";
              }
              
              ++algebraicOptNum;
              deadInsts.push_back(i);
            }
          }
        }
      }

      deleteDeadInsts(deadInsts);
    }

    void strength(Function &F) {
      std::vector<Instruction*> deadInsts;
      
      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); 
          i != ie; ++i) {
          if (i->getNumOperands() == 2) {

            long constVal1, constVal2;
            Value* opd1 = i->getOperand(0);
            Value* opd2 = i->getOperand(1);

            // get int64 signed for ReplaceInstWithValue
            if (isa<ConstantInt>(opd1)) {
              constVal1 = dyn_cast<ConstantInt>(opd1)->getSExtValue();
            }
            if (isa<ConstantInt>(opd2)) {
              constVal2 = dyn_cast<ConstantInt>(opd2)->getSExtValue();
            }
            bool strengthFlag = true;
            switch (i->getOpcode()) { 
              // ignore Add or Sub

              case Instruction::Mul:
                if (isa<ConstantInt>(opd1) && getShift(constVal1) != -1) {
                  // 2^n * x => x << n
                  Value* v = ConstantInt::getSigned(i->getType(), getShift(constVal1));
                  i->replaceAllUsesWith(BinaryOperator::Create(
                    Instruction::Shl, opd2, v, "shl", i));
                } else if (isa<ConstantInt>(opd2) && getShift(constVal2) != -1) { 
                  // x * 2^n => x << n
                  Value* v = ConstantInt::getSigned(i->getType(), getShift(constVal2));
                  i->replaceAllUsesWith(BinaryOperator::Create(
                    Instruction::Shl, opd1, v, "shl", i));
                } else {
                  strengthFlag = false;
                }
                break;
              case Instruction::SDiv:
                if (isa<ConstantInt>(opd2) && getShift(constVal2) != -1) { 
                  Value* v = ConstantInt::getSigned(i->getType(), getShift(constVal2));
                  // x / 2^n => x >> n
                  i->replaceAllUsesWith(BinaryOperator::Create(
                    Instruction::LShr, opd1, v, "lshr", i));
                } else {
                  strengthFlag = false;
                }
                break;
              default:
                strengthFlag = false;
                break;
            }
            if (strengthFlag) {
              if (DEBUG) {
                outs() << "[ST] ";
                i->print(errs());
                outs() << "\n";
              }
              
              ++strengthOptNum;
              deadInsts.push_back(i);
            }
          }
        }
      }

      deleteDeadInsts(deadInsts);
    }

    bool runOnFunction(Function &F) override {
      if (CF)
        constfold(F);
      if (AL)
        algebraic(F);
      if (ST)
        strength(F);
      
      outs() << "Transformations applied:" << "\n";
      outs() << "  Algebraic identities: " << algebraicOptNum << "\n";
      outs() << "  Constant folding: " << constfoldOptNum << "\n";
      outs() << "  Strength reduction: " << strengthOptNum << "\n";

      return false;
    }

  };
}


// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LocalOpts::ID = 0;
static RegisterPass<LocalOpts> X("local-opts", "15745: LocalOpts", false, false);


