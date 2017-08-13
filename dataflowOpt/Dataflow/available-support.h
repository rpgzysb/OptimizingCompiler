// 15-745 S16 Assignment 2: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef __AVAILABLE_SUPPORT_H__
#define __AVAILABLE_SUPPORT_H__

#include <string>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
  std::string getShortValueName(Value * v);

  class Expression {
  public:
    Value * v1;
    Value * v2;
    Instruction::BinaryOps op;
    Expression (Instruction * I);
    Expression(const Expression& e);
    bool containsOperand(const Value* op) const;
    bool operator== (const Expression &e2) const;
    bool operator< (const Expression &e2) const;
    std::string toString() const;
  };

  void printSet(std::vector<Expression> * x);
}

#endif
