// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef _DOMINATORS_ANALYSIS_
#define _DOMINATORS_ANALYSIS_

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

	class DominatorAnalysis : public ForwardAnalysis<std::set<std::string>>
	{
	public:
		using FlowSet = std::set<std::string>;

		DominatorAnalysis(Function& fblock);

		FlowSet flowFunction(FlowSet& before, BasicBlock& bb);
		
		FlowSet blockInitialization();
		FlowSet entryInitialization();
		FlowSet copy(FlowSet& from);
		FlowSet merge(FlowSet& src1, FlowSet& src2);
		bool equalState(FlowSet& src1, FlowSet& src2);

		// all blocks names
		std::set<std::string> all_blocks;
	};

}



#endif