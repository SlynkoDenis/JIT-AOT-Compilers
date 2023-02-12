#ifndef JIT_AOT_COMPILERS_COURSE_INLINING_H_
#define JIT_AOT_COMPILERS_COURSE_INLINING_H_

#include "dumper/DummyDumper.h"
#include "dumper/EventDumper.h"
#include "Graph.h"
#include "PassBase.h"


namespace ir {
class InliningPass : public OptimizationPassBase {
public:
    InliningPass(Graph *graph,
                 size_t maxCalleeInstrs,
                 size_t maxInstrsAfterInlining,
                 bool shouldDump = true)
        : OptimizationPassBase(graph),
          maxCalleeInstrs(maxCalleeInstrs),
          maxInstrsAfterInlining(maxInstrsAfterInlining)
    {
        ASSERT(maxCalleeInstrs < maxInstrsAfterInlining);
        if (shouldDump) {
            dumper = utils::dumper::EventDumper::AddDumper(graph->GetAllocator(), PASS_NAME).second;
        } else {
            dumper = utils::dumper::EventDumper::AddDumper<utils::dumper::DummyDumper>(
                graph->GetAllocator(), utils::dumper::DummyDumper::DUMPER_NAME).second;
        }
    }
    ~InliningPass() noexcept override = default;

    void Run() override;

private:
    // Returns a pointer to graph to be inlined if inlining is feasible to do, nullptr otherwise.
    const Graph *canInlineFunction(CallInstruction *call, size_t callerInstrsCount);

    void doInlining(CallInstruction *call, Graph *callee);

    // Connects caller's arguments directly into their usages in callee's DFG.
    void propagateArguments(CallInstruction *call, Graph *callee);

    // Connect callee's return value directly into its usages in caller's DFG.
    // Simply returns return in case of RETVOID
    void propagateReturnValue(CallInstruction *call, Graph *callee, BasicBlock *postCallBlock);

    void removeVoidReturns(Graph *callee);
    void inlineReadyGraph(Graph *callee, BasicBlock *callBlock, BasicBlock *postCallBlock);

    // Links all callee's basic blocks to the resulting caller graph.
    void relinkBasicBlocks(Graph *callerGraph, Graph *calleeGraph);

private:
    static constexpr const char *PASS_NAME = "inlining";

private:
    size_t maxCalleeInstrs;
    size_t maxInstrsAfterInlining;

    utils::dumper::EventDumper *dumper;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INLINING_H_
