// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream>
#include <queue>
#include <list>
#include <vector>
#include <set>
#include <functional>

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "DominatorAnalysis.h"

using namespace llvm;

namespace llvm {

	DominatorAnalysis::DominatorAnalysis(Function& fblock) : 
	ForwardAnalysis<std::set<std::string>>{fblock}
	{
		// collect all names of blocks
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			all_blocks.insert(bb->getName().str());
		}
		runAnalysis(fblock);
	}

	DominatorAnalysis::FlowSet DominatorAnalysis::blockInitialization()
	{
		FlowSet init{all_blocks};
		return std::move(init);
	}

	DominatorAnalysis::FlowSet DominatorAnalysis::entryInitialization()
	{
		FlowSet entry{};
		return entry;
	}

	DominatorAnalysis::FlowSet DominatorAnalysis::copy(FlowSet& from)
	{
		FlowSet dest{from};
		return std::move(dest);
	}

	DominatorAnalysis::FlowSet DominatorAnalysis::merge(FlowSet& src1, FlowSet& src2)
	{
		FlowSet dest{};
		std::set_intersection(src1.begin(), src1.end(),
							src2.begin(), src2.end(),
							std::inserter(dest, dest.begin()));
		return std::move(dest);
	}

	bool DominatorAnalysis::equalState(FlowSet& src1, FlowSet& src2)
	{
		return src1 == src2;	
	}

	DominatorAnalysis::FlowSet DominatorAnalysis::flowFunction(FlowSet& before, BasicBlock& bb)
	{
		FlowSet dest{before};

		std::string bb_name{bb.getName().str()};
		dest.insert(bb_name);
		return std::move(dest);
	}

}