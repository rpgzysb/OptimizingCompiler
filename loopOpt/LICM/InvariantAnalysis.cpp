// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/IntrinsicInst.h"

#include <map>
#include <string>

#include "InvariantAnalysis.h"

using namespace llvm;

/*
 * This is the implementation of the invariant expression analysis. 
*/
namespace llvm {

	// Method to analyze a loop with depth 1.
	InvariantAnalysis::FlowSet InvariantAnalysis::analyzeSingleLoop(Loop* lp)
	{
		// create an empty invariant set
		// and then append variables as we analyze.
		FlowSet invariantSet{};

		// initialization of the invariant set
		// collect all variables' names
		for (auto bbit = lp->block_begin(); bbit != lp->block_end(); ++bbit) {
			auto bb = *bbit;
			for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
				// definition
				if (ins->hasName()) {
					invariantSet[ins->getName().str()] = false;
				}
				// operands
				for (auto ops = ins->op_begin(); 
					ops != ins->op_end(); ++ops) {
					Value* operand = *ops;
					if (isa<Instruction>(operand) || isa<Argument>(operand)) {
						if (operand->hasName()) {
							invariantSet[operand->getName().str()] = false;
						}
					}
				}
			}
		}

		// after collecting
		// begin to iterate until we have a fixed point
		bool change = true;
		while (change) {
			change = false;
			// iterate through all basic blocks in a loop
			for (auto bbit = lp->block_begin(); bbit != lp->block_end(); ++bbit) {
				auto bb = *bbit;
				// all instructions
				for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
					// an expression is invariant if
					// (1) all variables are invariant
					// (2) the instruction itself is safe to transform
					if (this->invariantExpression(lp, ins, invariantSet) && 
						this->isInvariant(ins)) {
						std::string ins_name{ins->getName().str()};
						// put into the set
						if (!invariantSet[ins_name]) {
							invariantSet[ins_name] = true;
							change = true;
						}
					}
				}
			}
		}

		return invariantSet;
	}

	// Determine whether an instruction is invariant
	bool InvariantAnalysis::isInvariant(Instruction* I)
	{
		return isSafeToSpeculativelyExecute(I) && 
			!I->mayReadFromMemory() &&
			!isa<LandingPadInst>(I);
	}

	// Determine whether an operand is a constant
	bool InvariantAnalysis::isOperandConstant(Value* val)
	{
		return isa<ConstantInt>(val);
	}

	// Determine whether an operand is invariant inside the loop
	bool InvariantAnalysis::isInvariantInsideLoop(Loop *lp,
						Value* val, FlowSet& invariantSet)
	{
		return invariantSet[val->getName().str()];
	}

	// Determine whether an operand is defined outside the loop
	bool InvariantAnalysis::definedOutsideLoop(Loop* lp, Value* val)
	{
		// argument is defined at the entry point
		if (isa<Argument>(val)) {return true;}
		else if (isa<Instruction>(val)) {
			// see if defined outside the loop
			return (!lp->contains(cast<Instruction>(val)));
		}
		return false;
	}

	// A variable is invariant if
	// (1) constant
	// (2) defined outside the loop
	// (3) invariant inside the loop
	bool InvariantAnalysis::satisfyInvariantCondition(Loop* lp, 
						Value* val, FlowSet& invariantSet)
	{
		return isOperandConstant(val) || 
			isInvariantInsideLoop(lp, val, invariantSet) ||
			definedOutsideLoop(lp, val);
	}

	// An expression is invariant if all variables are invariant
	bool InvariantAnalysis::invariantExpression(Loop* lp, Instruction* ins,
						FlowSet& invariantSet)
	{

		for (auto ops = ins->op_begin(); ops != ins->op_end(); ++ops) {
			Value* val = *ops;
				if (!satisfyInvariantCondition(lp, val, invariantSet)) {
					return false;
				}
		}
		return true;
	}
}