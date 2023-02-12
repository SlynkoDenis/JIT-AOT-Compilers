#ifndef JIT_AOT_COMPILERS_COURSE_PASS_BASE_H_
#define JIT_AOT_COMPILERS_COURSE_PASS_BASE_H_

#include "Graph.h"


namespace ir {
class OptimizationPassBase {
public:
    explicit OptimizationPassBase(Graph *graph) : graph(graph)
    {
        ASSERT(graph);
    }
    NO_COPY_SEMANTIC(OptimizationPassBase);
    NO_MOVE_SEMANTIC(OptimizationPassBase);
    virtual ~OptimizationPassBase() noexcept = default;

    virtual void Run() = 0;

protected:
    Graph *graph;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_PASS_BASE_H_
