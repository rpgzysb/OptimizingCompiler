// 15-745 S16 Assignment 2: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef _LIVENESS_ANALYSIS_
#define _LIVENESS_ANALYSIS_


#include <stdio.h>
#include <iostream>
#include <queue>
#include <list>
#include <vector>
#include <set>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"

#include "dataflow.hpp"

using namespace llvm;

namespace llvm {

	// This is the implementation of liveness dataflow analysis using the framework.
	// It uses map of string key with boolean value as the flow set.

	class LivenessAnalysis : public BackwardBranchAnalysis<std::map<std::string, bool>>
	{
	public:
		using FlowSet = std::map<std::string, bool>;

		LivenessAnalysis(Function& fblock);
		
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
