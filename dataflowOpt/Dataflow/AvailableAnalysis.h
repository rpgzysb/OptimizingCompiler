// 15-745 S16 Assignment 2: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef _AVAILABLE_ANALYSIS_
#define _AVAILABLE_ANALYSIS_


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
#include "available-support.h"

using namespace llvm;


namespace llvm {

	// This is the implementation of available dataflow analysis using the framework.
	// It uses map of expression key with boolean value as the flow set.
	class AvailableAnalysis : public ForwardAnalysis<std::map<Expression, bool>>
	{
	public:
		using FlowSet = std::map<Expression, bool>;

		AvailableAnalysis(Function& fblock);
		
		FlowSet flowFunction(FlowSet& before, BasicBlock& bb);
		
		FlowSet blockInitialization();
		FlowSet entryInitialization();
		FlowSet copy(FlowSet& from);
		FlowSet merge(FlowSet& src1, FlowSet& src2);
		bool equalState(FlowSet& src1, FlowSet& src2);

		// We need to collect information for all instruction lines
		std::vector<FlowSet> getInstructionStates(std::string bb_name);
		std::map<std::string, std::vector<FlowSet>> ins_states; 

		std::set<Expression> all_exps;
	};

}


#endif
