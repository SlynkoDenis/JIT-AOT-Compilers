#ifndef JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_
#define JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_

#include "Loop.h"
#include "macros.h"
#include <vector>


namespace ir {
class LoopAnalyzer final {
public:
    void Analyze(Graph *graph);
    // TODO: destructor to delete loops

private:
    void resetStructs(size_t bblocksCount);

    void collectBackEdges(Graph *graph);
    void populateLoops();
    void buildLoopTree(Graph *graph);

    void dfsBackEdgesSearch(BasicBlock *bblock);
    void addLoopInfo(BasicBlock *header, BasicBlock *backEdgeSource);

    void populateReducibleLoop(Loop *loop);
    void dfsPopulateLoops(Loop *loop, BasicBlock *bblock, DFSColors color);

    static bool isLoopIrreducible(const BasicBlock *header, const BasicBlock *backEdgeSource) {
        return !header->Domites(backEdgeSource);
    }

private:
    // TODO: may replace vector with some packed array
    uint32_t colorCounter;
    std::vector<DFSColors> dfsColors;

    size_t blockId;
    std::vector<BasicBlock *> dfsBlocks;

    std::vector<Loop *> loops;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LOOP_ANALYZER_H_
