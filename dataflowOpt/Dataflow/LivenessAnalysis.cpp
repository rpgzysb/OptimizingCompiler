// 15-745 S16 Assignment 2: dataflow.hpp
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

#include "LivenessAnalysis.h"

using namespace llvm;


namespace llvm {

	// constructor for livesness analysis, and it is a backward analysis
	LivenessAnalysis::LivenessAnalysis(Function& fblock) : 
	BackwardBranchAnalysis<std::map<std::string, bool>>{fblock}
	{
		// it goes over all basic blocks and collect all variable names
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			// current basic block name
			std::string bb_name{bb->getName().str()};

			// initialize global state
			ins_states[bb_name].resize(bb->size());

			// check the components of the instruction
			for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
				// if definition
				if (ins->hasName()) {
					all_vars.insert(ins->getName().str());
				}

				// traverse the operands
				for (auto ops = ins->op_begin(); ops != ins->op_end(); ++ops) {
					Value* val = *ops;
					// we only care about these two types
					if (isa<Instruction>(val) || isa<Argument>(val)) {
						if (val->hasName()) {
							all_vars.insert(val->getName().str());
						}
					}
				}

			}
		}
		// must call this method in constructor.
		runAnalysis(fblock);
	}

	// This is the method to get states of instructions within a basic block.
	std::vector<LivenessAnalysis::FlowSet> LivenessAnalysis::getInstructionStates(std::string bb_name)
	{
		return ins_states[bb_name];
	}

	// We initialize all blocks with empty set.
	LivenessAnalysis::FlowSet LivenessAnalysis::blockInitialization()
	{
		FlowSet init{};
		for (auto vname : all_vars) {
			init[vname] = false;
		}
		return std::move(init);
	}

	// We initialize the entry block as empty set.
	LivenessAnalysis::FlowSet LivenessAnalysis::entryInitialization()
	{
		FlowSet exit{};
		for (auto vname : all_vars) {
			exit[vname] = false;
		}
		return std::move(exit);
	}

	// We just copy down the state when we fall from one basic block
	// to the other
	LivenessAnalysis::FlowSet LivenessAnalysis::copy(FlowSet& from)
	{
		FlowSet dest{from};
		return std::move(dest);
	}

	// When we do the merge, we use union.
	LivenessAnalysis::FlowSet LivenessAnalysis::merge(FlowSet& src1, FlowSet& src2)
	{
		FlowSet dest{};
		for (auto vname : all_vars) {
			dest[vname] = src1[vname] || src2[vname];
		}
		return std::move(dest);
	}

	// We just iterate through the sets to determine whether they
	// are equal.
	bool LivenessAnalysis::equalState(FlowSet& src1, FlowSet& src2)
	{
		for (auto vname : all_vars) {
			if (src1[vname] != src2[vname]) {
				return false;
			}
		}
		return true;
	}

	// Flow function goes through all instructions.
	std::map<std::string, LivenessAnalysis::FlowSet> 
		LivenessAnalysis::flowFunction(FlowSet& before, BasicBlock& bb)
	{
		// the name of current basic block
		std::string bb_name{bb.getName().str()};
		// the OUT state
		std::map<std::string, FlowSet> dests{};
		BasicBlock* ptr_bb = &bb;
		// copy all states first, and modify accordingly
		for (auto pred_bb = pred_begin(ptr_bb); pred_bb != pred_end(ptr_bb); ++pred_bb) {
			dests[(*pred_bb)->getName().str()] = before;
		}
		// we are iterating in backward direction
		std::list<decltype(bb.begin())> reverse_ins{};
		for (auto ins = bb.begin(); ins != bb.end(); ++ins) {
			reverse_ins.push_front(ins);
		}
		// index for global state for all instructions
		size_t idx = 0;
		// iterate
		for (auto ins : reverse_ins) {
			// if it is definition
			if (ins->hasName()) {
				// kill all defined variables first
				before[ins->getName().str()] = false;
				for (auto& dvp : dests) {
					dvp.second[ins->getName().str()] = false;
				}
			}
			if (isa<PHINode>(ins)) {
				// dispatch information for basic blocks
				PHINode* phi_ins = cast<PHINode>(ins);
				for (auto bb = phi_ins->block_begin(); bb != phi_ins->block_end(); ++bb) {
					Value* val = phi_ins->getIncomingValueForBlock(*bb);
					if (isa<Instruction>(val) || isa<Argument>(val)) {
						// only propagate information for incoming basic block
						before[val->getName().str()] = true;
						dests[(*bb)->getName().str()][val->getName().str()] = true;
					}
				}
			}
			else {
				// normal instruction, generate all uses
				for (auto ops = ins->op_begin(); ops != ins->op_end(); ++ops) {
					Value* val = *ops;
					if (isa<Instruction>(val) || isa<Argument>(val)) {
						if (val->hasName()) {
							// enable use
							before[val->getName().str()] = true;
							for (auto& dvp : dests) {
								dvp.second[val->getName().str()] = true;
							}
						}
					}
				}
			}
			// write to the global state
			FlowSet curr_flow{before};
			ins_states[bb_name][idx] = std::move(curr_flow);
			idx++;
		}
		return std::move(dests);
	}
}