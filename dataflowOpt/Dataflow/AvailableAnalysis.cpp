// 15-745 S16 Assignment 2: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

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

#include "AvailableAnalysis.h"
#include "available-support.h"

using namespace llvm;

namespace llvm {

	// The constructor for available analysis class.
	// It initializes all global states, and collect all 
	// expressions
	AvailableAnalysis::AvailableAnalysis(Function& fblock) : 
	ForwardAnalysis<std::map<Expression, bool>>{fblock}
	{
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			// name of current basic block
			std::string bb_name{bb->getName().str()};
			// initialize global state
			ins_states[bb_name].resize(bb->size());
			// collect all expressions
			for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
				if (ins->isBinaryOp()) {
					all_exps.insert(Expression{ins});
				}
			}
		}
		// must call this method in constructor
		runAnalysis(fblock);
	}

	// get the global state for instructions within a basic block
	std::vector<AvailableAnalysis::FlowSet> AvailableAnalysis::getInstructionStates(std::string bb_name)
	{
		return ins_states[bb_name];
	}

	// We initialize all blocks to be universal set
	AvailableAnalysis::FlowSet AvailableAnalysis::blockInitialization() 
	{
		FlowSet init{};
		for (auto exp : all_exps) {
			init[exp] = true;
		}
		return std::move(init);
	}

	// We initialize the entry as empty set
	AvailableAnalysis::FlowSet AvailableAnalysis::entryInitialization()
	{
		FlowSet entry{};
		for (auto exp : all_exps) {
			entry[exp] = false;
		}
		return std::move(entry);
	}

	// We just copy through the state to the next block
	AvailableAnalysis::FlowSet AvailableAnalysis::copy(FlowSet& from)
	{
		FlowSet dest{from};
		return std::move(dest);
	}

	// We use intersection as the meet operator.
	AvailableAnalysis::FlowSet AvailableAnalysis::merge(FlowSet& src1, FlowSet& src2)
	{
		FlowSet dest{};
		for (auto exp : all_exps) {
			dest[exp] =  src1[exp] && src2[exp];
		}
		return std::move(dest);
	}

	// We just go through the states to see if they are equal
	bool AvailableAnalysis::equalState(FlowSet& src1, FlowSet& src2)
	{
		for (auto exp : all_exps) {
			if (src1[exp] != src2[exp]) {
				return false;
			}
		}
		return true;
	}

	// Runnning the flow fucntion
	AvailableAnalysis::FlowSet AvailableAnalysis::flowFunction(FlowSet& before, BasicBlock& bb)
	{
		// the OUT state
		FlowSet dest{before};
		// initialize the kill and gen set
		std::set<Expression> kill_set{};
		std::set<Expression> gen_set{}; 
		// name of current basic block
		std::string bb_name{bb.getName().str()};
		// the current flow state
		FlowSet curr_flow{before};
		// all states within this basic block
		std::vector<FlowSet> curr_ins_states{ins_states[bb_name]};

		int index = 0;
		for (auto ins = bb.begin(); ins != bb.end(); ++ins) {
			// only cares about binary operation	
			if (ins->isBinaryOp()) {
				// generate expressions that are used
				Expression e{ins};
				gen_set.insert(e);
				for (auto exp : all_exps) {
					// kill expressions that have been defined
					if (exp.containsOperand(ins)) {
						kill_set.insert(exp);
						curr_flow[exp] = false;
					}
				}
				// for a single instruction, gen goes later
				curr_flow[e] = true;

			}
			// write back to the global state
			ins_states[bb_name][index] = curr_flow;
			index++;
		}
		// for basic block kill first
		for (auto to_kill : kill_set) {
			dest[to_kill] = false;
		}
		// for basic block gen second
		for (auto to_gen : gen_set) {
			dest[to_gen] = true;
		}
		return std::move(dest);
	}
}	