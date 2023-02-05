#ifndef JIT_AOT_COMPILERS_COURSE_DCE_H_
#define JIT_AOT_COMPILERS_COURSE_DCE_H_

#include "Graph.h"
#include "macros.h"


namespace ir {
class DCEPass {
public:
    explicit DCEPass(Graph *graph, bool shouldDump = true) : graph(graph) {
        ASSERT(graph);
    }
    NO_COPY_SEMANTIC(DCEPass);
    NO_MOVE_SEMANTIC(DCEPass);
    virtual DEFAULT_DTOR(DCEPass);

    void Run();

private:
    Graph *graph;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DCE_H_
