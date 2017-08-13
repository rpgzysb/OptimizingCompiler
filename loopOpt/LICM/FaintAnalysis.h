// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef _FAINT_ANALYSIS_
#define _FAINT_ANALYSIS_

#include <iostream>
#include <string>
#include <set>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"

#include "dataflow.hpp"

using namespace llvm;

namespace llvm {

	// This is the faint analysis inherited frmo the data flow analysis framework
	class FaintAnalysis : public BackwardBranchAnalysis<std::map<std::string, bool>>
	{
	public:
		using FlowSet = std::map<std::string, bool>;

		FaintAnalysis(Function& fblock);
		
		std::map<std::string, FlowSet> flowFunction(FlowSet& before, BasicBlock& bb);
		
		FlowSet blockInitialization();
		FlowSet entryInitialization();
		FlowSet copy(FlowSet& from);
		FlowSet merge(FlowSet& src1, FlowSet& src2);
		bool equalState(FlowSet& src1, FlowSet& src2);

		// We need to collect information for all instruction lines
		std::vector<FlowSet> getInstructionStates(std::string bb_name);
		// the states of all the instrucation
		std::map<std::string, std::vector<FlowSet>> ins_states; 
		// the set of all variables in the function.
		std::set<std::string> all_vars;
	};

}

#endif
