// 15-745 S16 Assignment 2: dataflow.hpp
// Group: Xian Zhang(xianz), Yuzhong Zhang(yuzhongz)
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include <stdio.h>
#include <iostream>
#include <queue>
#include <list>
#include <vector>
#include <set>

#include "llvm/IR/Instructions.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"

using namespace llvm;

namespace llvm {

	
	// This is the dataflow analysis framework.
	// The class hierarchy is:
	// DataflowAnalysis(abstract) 
	//
	//		|--- SingleBranchAnalysis(abstract) 
	//
	//			 |--- ForwardAnalysis(abstract)		 
	//
	//		|--- MultipleBranchAnalysis(abstract)
	//
	//			 |--- BackwardBranchAnalysis(abstract)
	//
	// The DataflowAnalysis class requires a FlowSet type, which 
	// specify the type to record state information on each statement
	// The ForwardAnalysis/BackwardBranchAnalysis subclass will implement
	// the runAnalysis method to determine how the dataflow analysis
	// is actually happening.
	//
	// All concerete dataflow analysis classes should extend either
	// ForwardAnalysis or BackwardBranchAnalysis. 
	// All concerete dataflow analysis classes should provide the implentation
	// of flowFunction, blockInitialization, entryInitialization, copy, merge, equalState
	// methods.
	// The semantics of these functions are as below:
	// 
	// FlowSet flowFunction(FlowSet& before, const BasicBlock& bb)
	// 		This function looks at the current BasicBlock bb with the state
	//		before, and this function should return a FlowSet that records the state
	// 		after executing the current basic block.
	//
	// FlowSet blockInitialization()
	//		This function returns a FlowSet that records the initial state of all 
	//		In/Out blocks before doing any dataflow analysis except for the entry/exit
	// 		block.
	//
	// FlowSet entryInitialization()
	//		This function returns a FlowSet that records the initial state of the
	//		entry/exit block.
	//
	// FlowSet copy(FlowSet& from)
	// 		This function determines how the FlowSet from one block is copied to
	// 		a different block by returning the FlowSet.
	//
	// FlowSet merge(FlowSet& src1, FlowSet& src2)
	//		This function determines how FlowSets from two different branches are
	// 		merged together. This function is used to merge different branches.
	//
	// bool equalState(FlowSet& src1, FlowSet& src2)
	//		This function determines if two different states are the same. It is used
	// 		to determine whether we have reach a fixed point or not.
	//
	// This DataflowAnalysis framework uses the name of each basic block to distinguish
	// different basic blocks. Users may get the state after running the dataflow analysis
	// using the getFlowAfter(std::string bb_name) function by providing the name of the 
	// basic block.
	// Or the user can get all the states without providing any arguments.
	template <typename FlowSet>
	class DataflowAnalysis 
	{
	public:
		DataflowAnalysis(Function& fblock);
		virtual ~DataflowAnalysis() {}

		// Get gloabl states (the states for all the basic blocks)
		// <basic block name, basick block's dataflow state>
		std::map<std::string, FlowSet> getFlowAfter() { return state_after_curr; }

		// Get a basic block's state
		FlowSet getFlowAfter(std::string bb_name) { return state_after_curr[bb_name]; }

		// Providing the interface
		virtual FlowSet blockInitialization() = 0;
		virtual FlowSet entryInitialization() = 0;
		virtual FlowSet copy(FlowSet& from) = 0;
		virtual FlowSet merge(FlowSet& src1, FlowSet& src2) = 0;
		virtual bool equalState(FlowSet& src1, FlowSet& src2) = 0;

		virtual void runAnalysis(Function& fblock) = 0;
		
		FlowSet accumulateBranch(std::vector<FlowSet>& srcs);
		FlowSet initialize(std::string bb_name, std::string entry_block);

		bool allStateEqual();

		// the set of name of all basic blocks
		std::vector<std::string> bb_set;
		// the global state of last iteration
		std::map<std::string, FlowSet> state_before_last;
		std::map<std::string, FlowSet> state_after_last;
		// the global state of current iteration
		std::map<std::string, FlowSet> state_before_curr;
		std::map<std::string, FlowSet> state_after_curr;
	};


	// This is the contructor for DataflowAnalysis framework.
	// It also collects all the names of the basic blocks so we can get access to them
	// later.
	template <typename FlowSet>
	DataflowAnalysis<FlowSet>::DataflowAnalysis(Function& fblock)
	{
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			bb_set.push_back(bb->getName().str());
		}
	}

	// This is the function used to determine whether we have reached a fixed
	// point during iteration. It compares all the state after applying flow function
	// from last iteration and current iteration.
	template <typename FlowSet>
	bool DataflowAnalysis<FlowSet>::allStateEqual()
	{
		for (auto bb_name : bb_set) {
			if (!equalState(state_after_last[bb_name],
							state_after_curr[bb_name])) {
				return false;
			}
		}
		return true;
	}

	// This is the function to initialize each basic block state.
	template <typename FlowSet>
	FlowSet DataflowAnalysis<FlowSet>::initialize(std::string bb_name, std::string entry_block)
	{
		return (bb_name == entry_block) ? entryInitialization() : blockInitialization();
	}

	// This is the helper function to merge different branches. If there are more than
	// two branches, we need to use this function and is based on the merge function provided.
	// This is the internal merge function used in this framework.
	template <typename FlowSet>
	FlowSet DataflowAnalysis<FlowSet>::accumulateBranch(std::vector<FlowSet>& srcs)
	{
		FlowSet dest{ srcs[0] };
		
		for (int i = 1; i < srcs.size(); ++i) {
			dest = merge(dest, srcs[i]);
		}
		return std::move(dest);
	}

	// This is the single branch analysis class.
	// It abstract out the single branch dispatch semantics in dataflow analysis.
	template <typename FlowSet>
	class SingleBranchAnalysis : public DataflowAnalysis<FlowSet>
	{
	public:
		SingleBranchAnalysis(Function& fblock);
		virtual ~SingleBranchAnalysis() {}

		virtual FlowSet flowFunction(FlowSet& before, BasicBlock& bb) = 0;
	};

	// This is the constructor for SingleBranchAnalysis.
	template <typename FlowSet>
	SingleBranchAnalysis<FlowSet>::SingleBranchAnalysis(Function& fblock) : 
	DataflowAnalysis<FlowSet>{fblock}
	{}


	// This is the ForwardAnalysis class. It implements the runAnalysis function
	// by iterating through the basic block from the beginning to the end.
	template <typename FlowSet>
	class ForwardAnalysis : public SingleBranchAnalysis<FlowSet>
	{
	public:
		ForwardAnalysis(Function& fblock);
		virtual ~ForwardAnalysis() {}
		void runAnalysis(Function& fblock);

		std::string entry_block;
		using DataflowAnalysis<FlowSet>::bb_set;
		using DataflowAnalysis<FlowSet>::state_before_last;
		using DataflowAnalysis<FlowSet>::state_after_last;
		using DataflowAnalysis<FlowSet>::state_before_curr;
		using DataflowAnalysis<FlowSet>::state_after_curr;
	};


	// This is the constructor for ForwardAnalysis.
	// It initializes the entry block's name.
	template <typename FlowSet>
	ForwardAnalysis<FlowSet>::ForwardAnalysis(Function& fblock) : 
	SingleBranchAnalysis<FlowSet>{fblock},
	entry_block{fblock.begin()->getName().str()} 
	{}

	// This is the runAnalysis function implemented by ForwardAnalysis.
	// It iterates through the basic block from the beginning to the end.
	template <typename FlowSet>
	void ForwardAnalysis<FlowSet>::runAnalysis(Function& fblock)
	{
		// We first initializes all the states of basic blocks.
		for (auto bb_name : bb_set) {
			if (bb_name == entry_block) {
				state_before_last[bb_name] = this->entryInitialization();
				state_before_curr[bb_name] = this->entryInitialization();
			}
			else {
				state_before_last[bb_name] = this->blockInitialization();
				state_before_curr[bb_name] = this->blockInitialization();
			}
		}

		// We first copy all the states, and modify the state if there are
		// states changes.
		state_after_last = state_before_last;
		state_after_curr = state_before_curr;
		// run analysis on each block
		bool change = true;
		while (change) {
			change = false;
			// we update the state for last time
			// and we will change the current state as we 
			// iterate, and compare the change to determine whether
			// we have reached a fixed point or not.
			state_before_last = state_before_curr;
			state_after_last = state_after_curr;
			// iterate all basic blocks
			for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
				// current block index using string	
				std::string bb_name{ bb->getName().str() };
				// collect all states of the predecessors of the current basic block.
				std::vector<FlowSet> pred_bb_states{};
				for (auto pred_bb = pred_begin(bb); pred_bb != pred_end(bb); ++pred_bb) {
					pred_bb_states.push_back(state_after_curr[(*pred_bb)->getName().str()]);
				}

				// for entry/exit block, there is actually no need to merge
				FlowSet curr_state{};
				if (bb_name == entry_block || pred_bb_states.size() == 1) {
					curr_state = state_before_curr[bb_name];
				}
				// if this is not the entry/exit block, we may merge all predecessors
				else {
					curr_state = this->accumulateBranch(pred_bb_states);
				}
				// execute the current statement using flow function
				FlowSet next_state = this->flowFunction(curr_state, *bb);
				// change the current state
				state_after_curr[bb_name] = next_state;
				// pass the current state to all its successors
				for (auto next_bb = succ_begin(bb); next_bb != succ_end(bb); ++next_bb) {
						std::string next_bb_name{ next_bb->getName().str() };
						state_before_curr[next_bb_name] = this->copy(next_state);
				}
				// see if the state change
				if (!this->allStateEqual()) {
					change = true;
				}
			}
		}
	}
	
	// This is the MultipleBranchAnalysis class. It abstracts out the backward
	// analysis in SSA form IR, and we need to dispatch dataflow information 
	// according to the PHINode.
	template <typename FlowSet>
	class MultipleBranchAnalysis : public DataflowAnalysis<FlowSet>
	{
	public:
		MultipleBranchAnalysis(Function& fblock);
		virtual ~MultipleBranchAnalysis() {}

		virtual std::map<std::string, FlowSet> 
		flowFunction(FlowSet& before, BasicBlock& bb) = 0;

		// the dictionary of all sets of branches for a given basic block
		std::map<std::string, std::set<std::string>> branch_set;
		// the state branch out information for each basic block before applying
		// flow function
		std::map<std::string, std::map<std::string, FlowSet>> state_branch_before;
		// the state branch out information for each basic block after applying
		// flow function
		std::map<std::string, std::map<std::string, FlowSet>> state_branch_after;
	};

	// This is the constructor for MultipleBranchAnalysis class.
	// It collects all the availble branch names for a basic block.
	template <typename FlowSet>
	MultipleBranchAnalysis<FlowSet>::MultipleBranchAnalysis(Function& fblock) : 
	DataflowAnalysis<FlowSet>{fblock}
	{
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			std::string bb_name{bb->getName().str()};
			for (auto prev_bb = pred_begin(bb); prev_bb != pred_end(bb); ++prev_bb) {
				branch_set[bb_name].insert((*prev_bb)->getName().str());
			}
		}
	}

	// The is the backward branch analysis class. It implements the runAnalysis function
	template <typename FlowSet>
	class BackwardBranchAnalysis : public MultipleBranchAnalysis<FlowSet>
	{
	public:
		BackwardBranchAnalysis(Function& fblock);
		virtual ~BackwardBranchAnalysis() {}
		void runAnalysis(Function& fblock);

		std::string entry_block;
		using DataflowAnalysis<FlowSet>::bb_set;
		using DataflowAnalysis<FlowSet>::state_before_last;
		using DataflowAnalysis<FlowSet>::state_after_last;
		using DataflowAnalysis<FlowSet>::state_before_curr;
		using DataflowAnalysis<FlowSet>::state_after_curr;
		using MultipleBranchAnalysis<FlowSet>::branch_set;
		using MultipleBranchAnalysis<FlowSet>::state_branch_after;
		using MultipleBranchAnalysis<FlowSet>::state_branch_before;
	};

	// This is the constructor for BackwardBranchAnalysis class.
	template <typename FlowSet>
	BackwardBranchAnalysis<FlowSet>::BackwardBranchAnalysis(Function& fblock) :
	MultipleBranchAnalysis<FlowSet>{fblock},
	entry_block{bb_set[bb_set.size() - 1]}
	{}

	// This is the implementation for runAnalysis function.
	template <typename FlowSet>
	void BackwardBranchAnalysis<FlowSet>::runAnalysis(Function& fblock)
	{
		// initialize all states
		for (std::string bb_name : bb_set) {
			state_before_last[bb_name] = this->initialize(bb_name, entry_block);
			state_before_curr[bb_name] = this->initialize(bb_name, entry_block);
			// state in
			std::set<std::string> curr_branch_set{ branch_set[bb_name] };
			// state out
			for (std::string branch_name : curr_branch_set) {
				// initialize all different branches.
				state_branch_before[bb_name][branch_name] = this->initialize(bb_name, entry_block);
			}
		}
		// initialize all state after.
		state_after_last = state_before_last;
		state_after_curr = state_before_curr;
		// initialize all branch state after
		state_branch_after = state_branch_before;
		// collect all basic blocks in reverse order
		std::list<decltype(fblock.begin())> reverse_basic_block{};
		for (auto bb = fblock.begin(); bb != fblock.end(); ++bb) {
			reverse_basic_block.push_front(bb);
		}
		// run analysis on each block
		bool change = true;
		while (change) {
			change = false;
			// we update the state from last time, and we will
			// compare the state after the current iteration
			state_before_last = state_before_curr;
			state_after_last = state_after_curr;
			// iterator goes backward
			for (auto bb : reverse_basic_block) {
				// get the current basic block's name
				std::string bb_name{ bb->getName().str() };
				// collect states of all successors
				std::vector<FlowSet> succ_bb_states{};
				auto curr_branch_set{ branch_set[bb_name] };
				for (auto succ_bb = succ_begin(bb); succ_bb != succ_end(bb); ++succ_bb) {
					succ_bb_states.push_back(state_branch_after[succ_bb->getName().str()][bb_name]);
				}
				// if it is the entry block, no need to merge
				FlowSet curr_state =
					(bb_name == entry_block) ? state_before_curr[bb_name] : this->accumulateBranch(succ_bb_states);
				// execute the current statement
				std::map<std::string, FlowSet> next_states =
					this->flowFunction(curr_state, *bb);
				// pass all current state information to the next block
				for (auto prev_bb = pred_begin(bb); prev_bb != pred_end(bb); ++prev_bb) {
						std::string next_bb_name{ (*prev_bb)->getName().str() };
						state_before_curr[next_bb_name] = next_states[next_bb_name];
				}
				// see if state changes
				if (!this->allStateEqual()) {
					change = true;
				}
				// update all current state
				state_after_curr[bb_name] = std::move(curr_state);
				state_branch_after[bb_name] = std::move(next_states);
			}
		}	
	}

}

#endif
