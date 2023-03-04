#ifndef JIT_AOT_COMPILERS_COURSE_GRAPH_CHECKER_H_
#define JIT_AOT_COMPILERS_COURSE_GRAPH_CHECKER_H_

#include "PassBase.h"
#include "Traversals.h"
#include <vector>


namespace ir {
class GraphChecker : public PassBase {
public:
    explicit GraphChecker(Graph *graph) : PassBase(graph) {}
    NO_COPY_SEMANTIC(GraphChecker);
    NO_MOVE_SEMANTIC(GraphChecker);
    ~GraphChecker() noexcept override = default;

    bool Run() override {
        auto *g = graph;
        graph->ForEachBasicBlock([g](const BasicBlock *bblock) {
            VerifyControlAndDataFlowGraphs(bblock);
        });
        DFO::Traverse(graph, []([[maybe_unused]] const BasicBlock *bblock) {
            /* DFO dry run to check graph is properly connected */
        });
        return true;
    }

    static void VerifyControlAndDataFlowGraphs(const BasicBlock *bblock);
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_CHECKER_H_
