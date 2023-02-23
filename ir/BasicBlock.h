#ifndef JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_
#define JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_

#include "arena/ArenaAllocator.h"
#include "instructions/Instruction.h"
#include "macros.h"
#include <vector>


namespace ir {
class Graph;
class Loop;

class BasicBlock : public Markable {
public:
    using IdType = size_t;

    BasicBlock(Graph *graph);
    NO_COPY_SEMANTIC(BasicBlock);
    NO_MOVE_SEMANTIC(BasicBlock);
    virtual DEFAULT_DTOR(BasicBlock);

    IdType GetId() const {
        return id;
    }
    IdType GetSize() const {
        return instrsCount;
    }
    bool IsEmpty() const {
        return GetSize() == 0;
    }
    bool IsFirstInGraph() const;
    bool IsLastInGraph() const;
    bool HasNoPredecessors() const {
        return preds.empty();
    }
    bool HasNoSuccessors() const {
        return succs.empty();
    }
    utils::memory::ArenaVector<BasicBlock *> &GetPredecessors() {
        return preds;
    }
    const utils::memory::ArenaVector<BasicBlock *> &GetPredecessors() const {
        return preds;
    }
    utils::memory::ArenaVector<BasicBlock *> &GetSuccessors() {
        return succs;
    }
    const utils::memory::ArenaVector<BasicBlock *> &GetSuccessors() const {
        return succs;
    }
    InstructionBase *GetFirstInstruction() {
        return firstInst;
    }
    const InstructionBase *GetFirstInstruction() const {
        return firstInst;
    }
    InstructionBase *GetLastInstruction() {
        return lastInst;
    }
    const InstructionBase *GetLastInstruction() const {
        return lastInst;
    }
    PhiInstruction *GetFirstPhiInstruction() {
        return firstPhi;
    }
    const PhiInstruction *GetFirstPhiInstruction() const {
        return firstPhi;
    }
    PhiInstruction *GetLastPhiInstruction() {
        return lastPhi;
    }
    const PhiInstruction *GetLastPhiInstruction() const {
        return lastPhi;
    }

    BasicBlock *GetDominator() {
        return dominator;
    }
    const BasicBlock *GetDominator() const {
        return dominator;
    }
    utils::memory::ArenaVector<BasicBlock *> &GetDominatedBlocks() {
        return dominated;
    }
    const utils::memory::ArenaVector<BasicBlock *> &GetDominatedBlocks() const {
        return dominated;
    }
    bool Dominates(const BasicBlock *bblock) const;

    Loop *GetLoop() {
        return loop;
    }
    const Loop *GetLoop() const {
        return loop;
    }

    Graph *GetGraph() {
        return graph;
    }
    const Graph *GetGraph() const {
        return graph;
    }

    void SetId(IdType newId) {
        id = newId;
    }
    void AddPredecessor(BasicBlock *bblock);
    void AddSuccessor(BasicBlock *bblock);
    void RemovePredecessor(BasicBlock *bblock);
    void RemoveSuccessor(BasicBlock *bblock);
    void ReplaceSuccessor(BasicBlock *prevSucc, BasicBlock *newSucc);

    void SetDominator(BasicBlock *newIDom) {
        dominator = newIDom;
    }
    void AddDominatedBlock(BasicBlock *bblock) {
        dominated.push_back(bblock);
    }

    void SetLoop(Loop *newLoop) {
        loop = newLoop;
    }

    void SetGraph(Graph *g) {
        graph = g;
    }

    void PushForwardInstruction(InstructionBase *instr);
    void PushBackInstruction(InstructionBase *instr);
    void InsertBefore(InstructionBase *before, InstructionBase *target);
    void InsertAfter(InstructionBase *after, InstructionBase *target);
    void UnlinkInstruction(InstructionBase *target);
    void ReplaceInstruction(InstructionBase *prevInstr, InstructionBase *newInstr);

    // implemented as BasicBlock's method to be able to directly access internal fields
    std::pair<BasicBlock *, BasicBlock *> SplitAfterInstruction(InstructionBase *instr,
                                                                bool connectAfterSplit);

    // TODO: might make this method protected & friend with Graph
    BasicBlock *Copy(
        Graph *targetGraph,
        utils::memory::ArenaUnorderedMap<InstructionBase::IdType, InstructionBase *> *instrsTranslation) const;

    NO_NEW_DELETE;

public:
    // STL compatible iterator
    template <typename T>
    class Iterator {
    public:
        // iterator traits
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::bidirectional_iterator_tag;

        Iterator() : curr(nullptr) {}
        explicit Iterator(T instr) : curr(instr) {}
        Iterator &operator++() {
            curr = curr->GetNextInstruction();
            return *this;
        }
        Iterator operator++(int) {
            auto retval = *this;
            ++(*this);
            return retval;
        }
        Iterator &operator--() {
            curr = curr->GetPrevInstruction();
            return *this;
        }
        Iterator operator--(int) {
            auto retval = *this;
            --(*this);
            return retval;
        }
        bool operator==(const Iterator &other) const {
            return curr == other.curr;
        }
        value_type operator*() {
            return curr;
        }

    private:
        T curr;
    };

    auto begin() {
        InstructionBase *instr = GetFirstPhiInstruction();
        return Iterator{instr != nullptr ? instr : GetFirstInstruction()};
    }
    auto begin() const {
        const InstructionBase *instr = GetFirstPhiInstruction();
        return Iterator{instr != nullptr ? instr : GetFirstInstruction()};
    }
    auto end() {
        return Iterator<decltype(GetLastInstruction())>{nullptr};
    }
    auto end() const {
        return Iterator<decltype(GetLastInstruction())>{nullptr};
    }
    auto size() const {
        return GetSize();
    }

public:
    static constexpr IdType INVALID_ID = static_cast<IdType>(-1);

private:
    template <bool PushBack>
    void pushInstruction(InstructionBase *instr);

    void pushPhi(InstructionBase *instr);

    void replaceInControlFlow(InstructionBase *prevInstr, InstructionBase *newInstr);

private:
    IdType id = INVALID_ID;

    utils::memory::ArenaVector<BasicBlock *> preds;
    // for conditional branches the first successor must correspond to true branch
    utils::memory::ArenaVector<BasicBlock *> succs;

    size_t instrsCount = 0;

    PhiInstruction *firstPhi = nullptr;
    PhiInstruction *lastPhi = nullptr;
    InstructionBase *firstInst = nullptr;
    InstructionBase *lastInst = nullptr;

    BasicBlock *dominator = nullptr;
    utils::memory::ArenaVector<BasicBlock *> dominated;

    Loop *loop = nullptr;

    Graph *graph = nullptr;
};

static_assert(std::input_or_output_iterator<BasicBlock::Iterator<InstructionBase*>>);
static_assert(std::ranges::range<BasicBlock>);
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_
