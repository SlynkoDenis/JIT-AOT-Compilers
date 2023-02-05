#ifndef JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_
#define JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_

#include "Loop.h"
#include "macros.h"
#include <vector>


namespace ir {
class LoopAnalyzer final {
public:
    void Analyze(Graph *graph);

private:
    void resetStructs(Graph *graph);

    void collectBackEdges(Graph *graph);
    void populateLoops();
    void buildLoopTree(Graph *graph);

    void dfsBackEdgesSearch(BasicBlock *bblock, ArenaAllocator *const allocator);
    void addLoopInfo(BasicBlock *header, BasicBlock *backEdgeSource,
                     ArenaAllocator *const allocator);

    void populateReducibleLoop(Loop *loop);
    void dfsPopulateLoops(Loop *loop, BasicBlock *bblock, DFSColors color);

    static bool isLoopIrreducible(const BasicBlock *header, const BasicBlock *backEdgeSource) {
        return !header->Dominates(backEdgeSource);
    }

private:
    // TODO: may replace vector with some packed array
    uint32_t colorCounter = 0;
    ArenaVector<DFSColors> *dfsColors = nullptr;

    size_t blockId = 0;
    ArenaVector<BasicBlock *> *dfsBlocks = nullptr;

    ArenaVector<Loop *> *loops = nullptr;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_
