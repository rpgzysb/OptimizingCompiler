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
#include "llvm/Transforms/Utils/LoopUtils.h"

#include <map>
#include <string>

#include "InvariantAnalysis.h"
#include "DominatorAnalysis.h"

using namespace llvm;

namespace {

	class LoopInvariant : public LoopPass 
	{
	public:
		static char ID;
		// this is the type of the dominator set 
		// among the program
		using DominatorSet = std::map<std::string, DominatorAnalysis::FlowSet>;

		LoopInvariant() : LoopPass(ID) {}

		virtual bool runOnLoop(Loop* lp, LPPassManager& LPM)
		{
			// get the function block of this loop
			BasicBlock* bb_begin{*(lp->block_begin())};
			Function *fblock = bb_begin->getParent();
			// begin to analyze the dominators information
			DominatorAnalysis da{*fblock};
			DominatorSet dominators{da.getFlowAfter()};
			// hoist the loop
			hoistLoopInvariant(dominators, lp, LPM);
			return false;
		}

		// Determine whether the current block dominates all exits
		bool dominatesAll(std::set<std::string>& exits, std::string block, 
			DominatorSet& da)
		{
			DominatorAnalysis::FlowSet block_dominate_set{da[block]};
			for (auto e : exits) {
				if (block_dominate_set.find(e) == block_dominate_set.end()) {return false;}
			}
			return true;
		}

		// Hoist all loop invariants out to the preheader
		void hoistLoopInvariant(DominatorSet& da, Loop* lp, LPPassManager& LPM) 
		{
			if (lp->getLoopDepth() > 1) {
				// depth > 1
				// recursively call hoist method to transform the loop
				std::vector<Loop*> subLoops = lp->getSubLoops();
				
				for (Loop* sublp : subLoops) {
					hoistLoopInvariant(da, sublp, LPM);
				}
			}

			// the inner most loop 
			// transform the loop here
			BasicBlock* preheader = lp->getLoopPreheader();

			// we may ignore the loop that the built-in pass
			// is unable to insert a preheader
			if (preheader != nullptr) {
				// cache preheader
				std::string preheader_name{preheader->getName().str()};
				// collect all exits
				std::set<std::string> all_exits{};
				for (auto bbit = lp->block_begin();
					bbit != lp->block_end(); ++bbit) {
					auto bb = *bbit;
					std::string bb_name{bb->getName().str()};

					if (lp->isLoopExiting(bb)) {
						all_exits.insert(bb_name);
					}
				}

				// create a invariant expression analysis
				InvariantAnalysis ia{};
				InvariantAnalysis::FlowSet invSet{ia.analyzeSingleLoop(lp)};

				// iterate through all basic blocks of a loop
				for (auto bbit = lp->block_begin(); 
					bbit != lp->block_end(); ++bbit) {
					auto bb = *bbit;
					std::string bb_name{bb->getName().str()};
					// the set of instructions to be inserted into the preheader
					std::list<Instruction*> to_be_insert{};
					// determine whether an instrution has been processed already
					std::set<std::string> added{};

					if (dominatesAll(all_exits, bb_name, da)) {
						// dominated by the preheader
						for (auto ins = bb->begin(); ins != bb->end(); ++ins) {
							if (ins->hasName()) {
								// assignment instruction
								std::string ins_name{ins->getName().str()};
								if (invSet[ins_name]) {
									// we find the instruction in 
									// invariant set
									if (added.find(ins_name) == added.end()) {
										to_be_insert.push_back(ins);
										added.insert(ins_name);
									}
								}
							}
						}
					}
					// get the last instruction of the preheader block
					Instruction* terminate_ins = &(preheader->getInstList().back());
					// insert the instructions before the terminator 
					// such that they are executed accordingly at the end of
					// the preheader basic block
					for (auto ins : to_be_insert) {
						ins->moveBefore(terminate_ins);
					}

				}
			}
		}

		virtual void getAnalysisUsage(AnalysisUsage& AU) const
		{
			AU.setPreservesCFG();
		}
	};

	char LoopInvariant::ID = 0;
	RegisterPass<LoopInvariant> X("loop-invariant-code-motion", "15745 LoopInvariant");
}