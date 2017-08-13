# OptimizingCompiler
CMU 15745 with Xian Zhang

This project contains three kinds of optimization passes written using LLVM framework for CMU 15745 with Xian Zhang.
In order to make the dataflow analysis work, we implemented a dataflow analysis framework using LLVM framework under dataflowOpt folder.
The three kinds of optimizations include:

(1) Scalar Optimization: algebraic identities, constant folding, strength reduction

(2) Dataflow Optimization: deadcode elimination, liveness analysis, common subexpression elimination

(3) Loop Optimization: loop invariant code motion
