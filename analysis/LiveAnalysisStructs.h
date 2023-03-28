#ifndef JIT_AOT_COMPILERS_COURSE_LIVE_ANALYSIS_STRUCTS_H_
#define JIT_AOT_COMPILERS_COURSE_LIVE_ANALYSIS_STRUCTS_H_

#include <algorithm>
#include "Graph.h"
#include <unordered_set>


namespace ir {
// Live range of the instruction represented as half-opened interval [begin, end)
class LiveRange {
public:
    using RangeType = std::size_t;

    LiveRange(RangeType b, RangeType e) : begin(b), end(e) {
        ASSERT(begin <= end);
    }
    LiveRange() = delete;
    DEFAULT_COPY_SEMANTIC(LiveRange);
    DEFAULT_MOVE_SEMANTIC(LiveRange);
    DEFAULT_DTOR(LiveRange);

    void SetBegin(RangeType b) {
        ASSERT(b < end);
        begin = b;
    }
    RangeType GetBegin() const {
        return begin;
    }
    RangeType GetEnd() const {
        return end;
    }

    bool Includes(const LiveRange &other) const {
        return begin <= other.begin && end >= other.end;
    }
    bool Intersects(const LiveRange &other) const {
        return (begin >= other.begin && begin < other.end)
            || (other.begin >= begin && other.begin < end);
    }
    LiveRange Union(const LiveRange &other) const {
        ASSERT(Intersects(other));
        return {std::min(begin, other.begin), std::max(end, other.end)};
    }

    constexpr bool LeftAdjacent(const LiveRange &other) const {
        return end == other.begin;
    }
    constexpr bool operator<(const LiveRange &other) const {
        return end <= other.begin;
    }
    constexpr bool operator<=(const LiveRange &other) const {
        return end <= other.end;
    }
    constexpr bool operator==(const LiveRange &other) const {
        return begin == other.begin && end == other.end;
    }

private:
    RangeType begin;
    RangeType end;
};

inline std::ostream &operator<<(std::ostream &os, const LiveRange &rng) {
    os << '<' << rng.GetBegin() << ',' << rng.GetEnd() << '>';
    return os;
}

class LiveIntervals {
public:
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    explicit LiveIntervals(const allocator_type &a) : ranges(a), liveNumber(0), instr(nullptr) {}
    LiveIntervals(LiveRange::RangeType liveNum, InstructionBase *in, std::pmr::memory_resource *memResource)
        : ranges(memResource), liveNumber(liveNum), instr(in) {}
    LiveIntervals(LiveRange::RangeType liveNum, InstructionBase *in, const allocator_type &a)
        : ranges(a), liveNumber(liveNum), instr(in) {}

    LiveIntervals(std::initializer_list<LiveRange> init, InstructionBase *in)
        : ranges(init, in->GetBasicBlock()->GetGraph()->GetMemoryResource()),
          liveNumber(init.begin()->GetBegin()),
          instr(in)
    {}
    LiveIntervals(std::initializer_list<LiveRange> init, InstructionBase *in, const allocator_type &a)
        : ranges(init, a), liveNumber(init.begin()->GetBegin()), instr(in) {}
    LiveIntervals() = delete;

    NO_COPY_SEMANTIC(LiveIntervals);
    LiveIntervals(const LiveIntervals &other, const allocator_type &a)
        : ranges(other.ranges, a),
          liveNumber(other.liveNumber),
          instr(other.instr)
    {}

    NO_MOVE_SEMANTIC(LiveIntervals);
    LiveIntervals(LiveIntervals &&other, const allocator_type &a)
        : ranges(std::move(other.ranges), a),
          liveNumber(other.liveNumber),
          instr(other.instr)
    {}

    virtual DEFAULT_DTOR(LiveIntervals);

    LiveRange::RangeType GetLiveNumber() const {
        return liveNumber;
    }

    InstructionBase *GetInstruction() {
        return instr;
    }
    const InstructionBase *GetInstruction() const {
        return instr;
    }

    LiveRange GetUpperRange() const {
        if (ranges.empty()) {
            return {0, 0};
        }
        return {ranges.back().GetBegin(), ranges.front().GetEnd()};
    }

    bool operator==(const LiveIntervals &other) const {
        if (ranges.size() != other.ranges.size()) {
            return false;
        }
        for (size_t i = 0, end = ranges.size(); i < end; ++i) {
            if (ranges[i] != other.ranges[i]) {
                return false;
            }
        }
        return true;
    }

    void SetBegin(LiveRange::RangeType begin);

    void AddRange(LiveRange::RangeType begin, LiveRange::RangeType end) {
        AddRange({begin, end});
    }
    void AddRange(const LiveRange &rng);

    auto begin() {
        return ranges.rbegin();
    }
    auto end() {
        return ranges.rend();
    }
    auto begin() const {
        return ranges.rbegin();
    }
    auto end() const {
        return ranges.rend();
    }

public:
    static constexpr size_t LIVE_RANGE_STEP = 2;

private:
    // live ranges sorted in descending order
    std::pmr::vector<LiveRange> ranges;
    LiveRange::RangeType liveNumber;
    InstructionBase *instr;
};

inline std::ostream &operator<<(std::ostream &os, const LiveIntervals &intervals) {
    ASSERT(intervals.GetInstruction());
    os << '#' << intervals.GetInstruction()->GetId() << '\t';
    for (const auto &iter : intervals) {
        os << iter << ' ';
    }
    return os;
}

class LiveSet {
public:
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    explicit LiveSet(const allocator_type &a) : instructions(a) {}
    explicit LiveSet(std::pmr::memory_resource *memResource) : instructions(memResource) {}

    NO_COPY_SEMANTIC(LiveSet);
    LiveSet(const LiveSet &other, const allocator_type &a) : instructions(other.instructions, a) {}

    NO_MOVE_SEMANTIC(LiveSet);
    LiveSet(LiveSet &&other, const allocator_type &a) : instructions(std::move(other.instructions), a) {}

    virtual DEFAULT_DTOR(LiveSet);

    void Add(InstructionBase *instr) {
        ASSERT(instr);
        instructions.emplace(instr);
    }
    bool Remove(InstructionBase *instr) {
        ASSERT(instr);
        return instructions.erase(instr);
    }

    LiveSet &Union(LiveSet &&other) {
        instructions.merge(std::move(other.instructions));
        return *this;
    }
    LiveSet &Union(const LiveSet &other) {
        instructions.insert(other.instructions.begin(), other.instructions.end());
        return *this;
    }

    auto begin() {
        return instructions.begin();
    }
    auto end() {
        return instructions.end();
    }
    auto begin() const {
        return instructions.begin();
    }
    auto end() const {
        return instructions.end();
    }

    allocator_type get_allocator() const noexcept {
        return instructions.get_allocator();
    }

private:
    std::pmr::unordered_set<InstructionBase *> instructions;
};

class BlockInfo {
public:
    using allocator_type = LiveSet::allocator_type;

    explicit BlockInfo(const allocator_type &a) : bblock(nullptr), liveRange(0, 0), liveSet(a) {}
    BlockInfo(BasicBlock *b, LiveRange range)
        : bblock(b),
          liveRange(range),
          liveSet(b->GetGraph()->GetMemoryResource())
    {}
    BlockInfo(BasicBlock *b, LiveRange range, const allocator_type &a)
        : bblock(b),
          liveRange(range),
          liveSet(a)
    {}

    DEFAULT_COPY_SEMANTIC(BlockInfo);
    BlockInfo(const BlockInfo &other, const allocator_type &a)
        : bblock(other.bblock),
          liveRange(other.liveRange),
          liveSet(other.liveSet, a)
    {}

    DEFAULT_MOVE_SEMANTIC(BlockInfo);
    BlockInfo(BlockInfo &&other, const allocator_type &a)
        : bblock(other.bblock),
          liveRange(other.liveRange),
          liveSet(std::move(other.liveSet), a)
    {}

    DEFAULT_DTOR(BlockInfo);

    BasicBlock *GetBlock() {
        return bblock;
    }
    const BasicBlock *GetBlock() const {
        return bblock;
    }
    void SetBlock(BasicBlock *b) {
        bblock = b;
    }

    LiveRange GetRange() {
        return liveRange;
    }
    const LiveRange &GetRange() const {
        return liveRange;
    }
    void SetRange(LiveRange range) {
        liveRange = range;
    }

    LiveSet &GetLiveSet() {
        return liveSet;
    }
    const LiveSet &GetLiveSet() const {
        return liveSet;
    }

    allocator_type get_allocator() const noexcept {
        return liveSet.get_allocator();
    }

private:
    BasicBlock *bblock;
    LiveRange liveRange;
    LiveSet liveSet;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LIVE_ANALYSIS_STRUCTS_H_
