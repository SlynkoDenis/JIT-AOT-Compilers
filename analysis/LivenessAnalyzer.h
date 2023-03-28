#ifndef JIT_AOT_COMPILERS_COURSE_LIVENESS_ANALYZER_H_
#define JIT_AOT_COMPILERS_COURSE_LIVENESS_ANALYZER_H_

#include <list>
#include "LiveAnalysisStructs.h"
#include "PassBase.h"


namespace ir {
class LivenessAnalyzer : public PassBase {
public:
    explicit LivenessAnalyzer(Graph *graph)
        : PassBase(graph),
          linearOrderedBlocks(graph->GetMemoryResource()),
          liveIntervals(graph->GetMemoryResource())
    {}
    NO_COPY_SEMANTIC(LivenessAnalyzer);
    NO_MOVE_SEMANTIC(LivenessAnalyzer);
    ~LivenessAnalyzer() noexcept override = default;

    bool Run() override;

    std::pmr::vector<LiveIntervals> &GetLiveIntervals() {
        return liveIntervals;
    }
    const std::pmr::vector<LiveIntervals> &GetLiveIntervals() const {
        return liveIntervals;
    }

private:
    void resetStructs();

    // Lineary orderes basic blocks, such that:
    // * all predecessors of a block are located before this block
    // * all blocks that are part of the same loop are contiguous
    // (C. Wimmer, "Linear Scan Register Allocation on SSA Form", 2010)
    void orderBlocks();
    void addIntoQueue(std::pmr::list<BasicBlock *> &remainedBlocks, BasicBlock *bblock);
    bool unvisitedForwardEdgesExist(BasicBlock *bblock, Marker visitedMarker);

    // Globally orderes instructions in linear order.
    LiveRange::RangeType orderInstructions(BasicBlock *bblock);

    void calculateLiveRanges(BlockInfo &info);
    void calculateInitialLiveSet(BlockInfo &info) const;

    BlockInfo &getBlockInfo(BasicBlock *bblock) {
        ASSERT((bblock) && bblock->GetId() < linearOrderedBlocks.size());
        auto &res = linearOrderedBlocks[bblock->GetId()];
        ASSERT(res.GetBlock());
        return res;
    }
    const BlockInfo &getBlockInfo(const BasicBlock *bblock) const {
        ASSERT((bblock) && bblock->GetId() < linearOrderedBlocks.size());
        auto &res = linearOrderedBlocks[bblock->GetId()];
        ASSERT(res.GetBlock());
        return res;
    }

    LiveIntervals &getLiveIntervals(InstructionBase *instr) {
        ASSERT((instr) && instr->GetLinearNumber() < liveIntervals.size());
        return liveIntervals[instr->GetLinearNumber()];
    }
    const LiveIntervals &getLiveIntervals(const InstructionBase *instr) const {
        ASSERT((instr) && instr->GetLinearNumber() < liveIntervals.size());
        return liveIntervals[instr->GetLinearNumber()];
    }

private:
    std::pmr::vector<BlockInfo> linearOrderedBlocks;
    std::pmr::vector<LiveIntervals> liveIntervals;

    LiveRange::RangeType rangeBegin = 0;
    size_t linearNumber = 0;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LIVENESS_ANALYZER_H_
