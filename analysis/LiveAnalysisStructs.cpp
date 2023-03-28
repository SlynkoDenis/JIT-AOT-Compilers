#include "LiveAnalysisStructs.h"


namespace ir {
void LiveIntervals::SetBegin(LiveRange::RangeType begin) {
    if (ranges.empty()) {
        ranges.emplace_back(begin, begin + LIVE_RANGE_STEP);
    } else {
        ASSERT(ranges.back().GetEnd() >= begin);
        ranges.back().SetBegin(begin);
    }
}

void LiveIntervals::AddRange(const LiveRange &rng) {
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
}   // namespace ir
