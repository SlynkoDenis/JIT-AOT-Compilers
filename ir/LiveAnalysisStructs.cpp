#include "Graph.h"
#include "LiveAnalysisStructs.h"


namespace ir {
LiveInterval::LiveInterval(std::initializer_list<LiveRange> init, InstructionBase *in)
    : ranges(init, in->GetBasicBlock()->GetGraph()->GetMemoryResource()),
      liveNumber(init.begin()->GetBegin()),
      instr(in)
{}

void LiveInterval::SetBegin(LiveRange::RangeType begin) {
    if (ranges.empty()) {
        ranges.emplace_back(begin, begin + LIVE_RANGE_STEP);
    } else {
        ASSERT(ranges.back().GetEnd() >= begin);
        ranges.back().SetBegin(begin);
    }
}

void LiveInterval::AddRange(const LiveRange &rng) {
    if (ranges.empty()) {
        ranges.push_back(rng);
    } else {
        auto &earliest = ranges.back();
        if (rng.LeftAdjacent(earliest)) {
            earliest.SetBegin(rng.GetBegin());
        } else if (rng < earliest) {
            ranges.push_back(rng);
        } else {
            ASSERT(earliest.Includes(rng));
        }
    }
}

void LiveInterval::AddLoopRange(const LiveRange &rng) {
    ASSERT(!ranges.empty());
    auto &earliest = ranges.back();
    ASSERT(earliest.GetBegin() == rng.GetBegin());
    earliest.SetEnd(rng.GetEnd());

    for (int i = ranges.size() - 2; i >= 0; --i) {
        auto &r = ranges[i];
        if (rng.Includes(r)) {
            ranges.erase(ranges.begin() + i);
        } else if (r.GetBegin() <= rng.GetEnd()) {
            ASSERT(r.GetEnd() > rng.GetEnd());
            earliest.SetEnd(r.GetEnd());
            ranges.erase(ranges.begin() + i);
            break;
        } else {
            break;
        }
    }
}

LiveInterval *LiveIntervals::AddLiveInterval(LiveRange::RangeType liveNum, InstructionBase *instr) {
    ASSERT(instr);
    instr->SetLinearNumber(liveIntervals.size());
    auto *info = utils::template New<LiveInterval>(
        instr->GetBasicBlock()->GetGraph()->GetMemoryResource(), liveNum, instr);
    liveIntervals.push_back(info);
    return info;
}
}   // namespace ir
