// 15-745 S16 Assignment 3: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>
#include <map>
#include <string>

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
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"

#include "FaintAnalysis.h"

using namespace llvm;

namespace llvm {

	FaintAnalysis::FaintAnalysis(Function& fblock) :
	BackwardBranchAnalysis<std::map<std::string, bool>>{fblock}
	{
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			// current block name
			std::string bb_name{bb->getName().str()};
			// states of all instructions
			ins_states[bb_name].resize(bb->size());
			// iterate through the instructions
			for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
				if (ins->hasName()) {
					all_vars.insert(ins->getName().str());
				}
				// collect all operands
				for (auto ops = ins->op_begin(); ops != ins->op_end(); ++ops) {
					Value* val = *ops;
					if (isa<Instruction>(val) || isa<Argument>(val)) {
						if (val->hasName()) {
							all_vars.insert(val->getName().str());
						}
					}
				}
			}
		}
		// must run the analysis in constructor
		runAnalysis(fblock);
	}

	// get the instruction of the current block
	std::vector<FaintAnalysis::FlowSet> FaintAnalysis::getInstructionStates(std::string bb_name)
	{
		return ins_states[bb_name];
	}

	FaintAnalysis::FlowSet FaintAnalysis::blockInitialization()
	{
		FlowSet init{};
		for (auto vname : all_vars) {
			init[vname] = true;
		}
		return std::move(init);
	}

	FaintAnalysis::FlowSet FaintAnalysis::entryInitialization() 
	{
		FlowSet init{};
		for (auto vname : all_vars) {
			init[vname] = true;
		}
		return std::move(init);
	}

	FaintAnalysis::FlowSet FaintAnalysis::copy(FlowSet& from)
	{
		FlowSet dest{from};
		return std::move(dest);
	}

	// merge function is set intersection
	FaintAnalysis::FlowSet FaintAnalysis::merge(FlowSet& src1, FlowSet& src2)
	{
		FlowSet dest{};
		for (auto vname : all_vars) {
			dest[vname] = src1[vname] && src2[vname];
		}
		return std::move(dest);
	}

	bool FaintAnalysis::equalState(FlowSet& src1, FlowSet& src2)
	{
		for (auto vname : all_vars) {
			if (src1[vname] != src2[vname]) {
				return false;
			}
		}
		return true;
	}

	// see if the variant is contained in the flow set
	bool containsInFlowSet(std::string vname, FaintAnalysis::FlowSet src)
	{
		return src[vname];
	}

	// see if the instruction is live to delete
	bool isLive(Instruction* I) {
		return isa<TerminatorInst>(I) || isa<DbgInfoIntrinsic>(I) ||
			isa<LandingPadInst>(I) || I->mayHaveSideEffects();
	}

	// backward branch flow functions
	std::map<std::string, FaintAnalysis::FlowSet>
		FaintAnalysis::flowFunction(FlowSet& before, BasicBlock& bb)
	{
		std::string bb_name{bb.getName().str()};

		std::map<std::string, FlowSet> dests{};
		BasicBlock* ptr_bb = &bb;

		// collect all predecessors
		for (auto pred_bb = pred_begin(ptr_bb); 
			pred_bb != pred_end(ptr_bb); ++pred_bb) {
			dests[(*pred_bb)->getName().str()] = before;
		}

		// backward direction
		std::list<decltype(bb.begin())> reverse_ins{};
		for (auto ins = bb.begin(); ins != bb.end(); ++ins) {
			reverse_ins.push_front(ins);
		}

		// index for global state for all instructions
		size_t idx = 0;
		// begin backward analysis
		for (auto ins : reverse_ins) {

			if (ins->hasName()) {
				// assignment instruction
				// x = e

				// kill first
				if (isLive(ins) || !containsInFlowSet(ins->getName().str(), before)) {
					// x not in Flowset
					// kill all vars in the right handside e
					
					for (auto ops = ins->op_begin(); 
						ops != ins->op_end(); ++ops) {
						Value* val = *ops;
						if (val->hasName()) {
							before[val->getName().str()] = false;
							for (auto& dvp : dests) {
								dvp.second[val->getName().str()] = false;
							}
						}
					} 
				}

				// gen second
				// itereate through the operands
				if (containsInFlowSet(ins->getName().str(), before)) {
					before[ins->getName().str()] = true;
					for (auto& dvp : dests) {
						dvp.second[ins->getName().str()] = true;
					}
				}
			}
			else {
				// use(x)
				// not an assignment form
				
				// kill all use(x)
				// no need to gen
				for (auto ops = ins->op_begin(); 
					ops != ins->op_end(); ++ops) {
					Value* val = *ops;

					if (isa<Instruction>(val) || isa<Argument>(val)) {
						if (val->hasName()) {
							before[val->getName().str()] = false;
							for (auto dvp : dests) {
								dvp.second[val->getName().str()] = false;
							}
						}
					}
				}
			}
			FlowSet curr_flow{before};
			ins_states[bb_name][idx] = std::move(curr_flow);
			idx++;
		}
		return std::move(dests);
	}
}		