#ifndef JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_
#define JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_

#include "Loop.h"
#include "macros.h"
#include <vector>


namespace ir {
class LoopAnalyzer final {
public:
    void Analyze(Graph *targetGraph);

private:
    void resetStructs(Graph *targetGraph);

    void collectBackEdges();
    void populateLoops();
    void buildLoopTree();

    void dfsBackEdgesSearch(BasicBlock *bblock, ArenaAllocator *const allocator);
    void addLoopInfo(BasicBlock *header, BasicBlock *backEdgeSource,
                     ArenaAllocator *const allocator);

    void populateReducibleLoop(Loop *loop);
    void dfsPopulateLoops(Loop *loop, BasicBlock *bblock);

    static bool isLoopIrreducible(const BasicBlock *header, const BasicBlock *backEdgeSource) {
        return !header->Dominates(backEdgeSource);
    }

private:
    Graph *graph = nullptr;

    Marker greyMarker;
    Marker blackMarker;

    size_t blockId = 0;
    ArenaVector<BasicBlock *> *dfsBlocks = nullptr;

    ArenaVector<Loop *> *loops = nullptr;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_
