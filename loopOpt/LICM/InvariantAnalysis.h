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

using namespace llvm;

namespace llvm {

	/*
	 * This is the class used to give the loop invariant code motion
	 * pass necessary information. It analyzes a given loop, and then 
	 * print out all invariant expressions.
	*/
	class InvariantAnalysis
	{
	public:
		// The type of invariant set
		using FlowSet = std::map<std::string, bool>;
		// constructor
		InvariantAnalysis() {}
		// method to analyze a loop
		FlowSet analyzeSingleLoop(Loop* lp);
	private:
		bool isInvariant(Instruction*);
		bool isOperandConstant(Value*);
		bool isInvariantInsideLoop(Loop*, Value*, FlowSet&);
		bool definedOutsideLoop(Loop*, Value*);
		bool satisfyInvariantCondition(Loop*, Value*, FlowSet&);
		bool invariantExpression(Loop*, Instruction*, FlowSet&);	
	};

}