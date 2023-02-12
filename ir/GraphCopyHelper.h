#ifndef JIT_AOT_COMPILERS_COURSE_GRAPH_COPY_HELPER_H_
#define JIT_AOT_COMPILERS_COURSE_GRAPH_COPY_HELPER_H_

#include "Graph.h"
#include "InstructionBuilder.h"


namespace ir {
using namespace utils::memory;

class GraphCopyHelper final {
public:
    GraphCopyHelper(const Graph *source) : source(source)
    {
        ASSERT(source);
    }
    NO_COPY_SEMANTIC(GraphCopyHelper);
    NO_MOVE_SEMANTIC(GraphCopyHelper);
    DEFAULT_DTOR(GraphCopyHelper);

    Graph *CreateCopy(Graph *copyTarget);

private:
    void reset(Graph *copyTarget);
    void dfoCopy(const BasicBlock *currentBBlock);
    void fixDFG();

private:
    const Graph *source;
    Graph *target;

    ArenaUnorderedMap<InstructionBase::IdType, InstructionBase *> *instrsTranslation = nullptr;
    ArenaUnorderedMap<BasicBlock::IdType, BasicBlock *> *visited = nullptr;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_COPY_HELPER_H_
