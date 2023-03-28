#include <algorithm>
#include "LivenessAnalyzer.h"
#include "LoopAnalyzer.h"


namespace ir {
bool LivenessAnalyzer::Run() {
    PassManager::Run<LoopAnalyzer>(graph);

    resetStructs();
    orderBlocks();
    std::for_each(
        linearOrderedBlocks.rbegin(),
        linearOrderedBlocks.rend(),
        [this](auto &i) { calculateLiveRanges(i); });
    return true;
}

void LivenessAnalyzer::resetStructs() {
    linearOrderedBlocks.reserve(graph->GetMaximumBlockId());
    linearOrderedBlocks.clear();
    rangeBegin = 0;
    linearNumber = 0;
}

void LivenessAnalyzer::orderBlocks() {
    auto visitedMarker = graph->GetNewMarker();

    std::pmr::list<BasicBlock *> remainedBlocks(graph->GetMemoryResource());
    remainedBlocks.push_back(graph->GetFirstBasicBlock());

    while (!remainedBlocks.empty()) {
        auto *bblock = remainedBlocks.front();
        ASSERT(bblock);
        remainedBlocks.pop_front();

        bblock->SetMarker(visitedMarker);
        auto rangeEnd = orderInstructions(bblock);
        linearOrderedBlocks.emplace_back(bblock, std::move(LiveRange{rangeBegin, rangeEnd}));
        rangeBegin = rangeEnd;

        for (auto *succ : bblock->GetSuccessors()) {
            if (succ->IsMarkerSet(visitedMarker) || unvisitedForwardEdgesExist(succ, visitedMarker)) {
                continue;
            }
            addIntoQueue(remainedBlocks, succ);
        }
    }

    ASSERT(linearOrderedBlocks.size() == graph->GetBasicBlocksCount());
    ASSERT(liveIntervals.size() == linearNumber);
}

LiveRange::RangeType LivenessAnalyzer::orderInstructions(BasicBlock *bblock) {
    ASSERT(bblock);

    auto range = rangeBegin;
    for (auto *instr : bblock->IteratePhi()) {
        instr->SetLinearNumber(linearNumber++);
        liveIntervals.emplace_back(range, instr);
    }
    for (auto *instr : bblock->IterateNonPhi()) {
        instr->SetLinearNumber(linearNumber++);
        range += LiveIntervals::LIVE_RANGE_STEP;
        liveIntervals.emplace_back(range, instr);
    }
    return range + LiveIntervals::LIVE_RANGE_STEP;
}

void LivenessAnalyzer::addIntoQueue(std::pmr::list<BasicBlock *> &remainedBlocks,
                                    BasicBlock *bblock) {
    ASSERT(bblock);
    auto *loop = bblock->GetLoop();
    ASSERT(loop);
    auto insertPos = std::find_if(remainedBlocks.begin(),
                                  remainedBlocks.end(),
                                  [loop](const BasicBlock *b) {
        return loop->IsIn(b->GetLoop());
    });
    remainedBlocks.insert(insertPos, bblock);
}

bool LivenessAnalyzer::unvisitedForwardEdgesExist(BasicBlock *bblock, Marker visitedMarker) {
    ASSERT(bblock);
    if (bblock->GetLoop()->IsIrreducible()) {
        return false;
    }

    // here predecessors are checked to ensure the first invariant of
    // `orderBlocks` method is satisfied
    auto preds = bblock->GetPredecessors();
    if (!bblock->IsLoopHeader()) {
        return std::any_of(preds.begin(), preds.end(), [visitedMarker](auto pred) {
            return !pred->IsMarkerSet(visitedMarker);
        });
    }
    return std::any_of(preds.begin(), preds.end(), [bblock, visitedMarker](auto pred) {
        return !bblock->Dominates(pred) && !pred->IsMarkerSet(visitedMarker);
    });
}

void LivenessAnalyzer::calculateLiveRanges(BlockInfo &info) {
    auto *bblock = info.GetBlock();
    if (bblock == nullptr) {
        return;
    }
    auto blockRange = info.GetRange();
    auto &liveSet = info.GetLiveSet();

    calculateInitialLiveSet(info);
    for (auto *instr : liveSet) {
        getLiveIntervals(instr).AddRange(blockRange);
    }

    auto blockRangeBegin = blockRange.GetBegin();
    for (auto *instr = bblock->GetLastInstruction();
         instr != nullptr && !instr->IsPhi();
         instr = instr->GetPrevInstruction())
    {
        auto &intervals = getLiveIntervals(instr);
        auto liveNumber = intervals.GetLiveNumber();
        intervals.SetBegin(liveNumber);
        liveSet.Remove(instr);

        if (!instr->HasInputs()) {
            continue;
        }
        auto *withInputs = instr->AsInputsInstruction();
        for (size_t i = 0, end = withInputs->GetInputsCount(); i < end; ++i) {
            auto *input = withInputs->GetInput(i).GetInstruction();
            liveSet.Add(input);
            getLiveIntervals(input).AddRange(blockRangeBegin, liveNumber);
        }
    }
    for (auto *phi : bblock->IteratePhi()) {
        liveSet.Remove(phi);
    }
}

void LivenessAnalyzer::calculateInitialLiveSet(BlockInfo &info) const {
    auto &liveSet = info.GetLiveSet();
    auto *bblock = info.GetBlock();

    for (auto *succ : bblock->GetSuccessors()) {
        liveSet.Union(getBlockInfo(succ).GetLiveSet());
        for (auto *phi : succ->IteratePhi()) {
            liveSet.Add(phi->ResolveInput(bblock).GetInstruction());
        }
    }
}
}   // namespace ir
