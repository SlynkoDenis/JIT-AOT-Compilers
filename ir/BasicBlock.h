#ifndef JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_
#define JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_

#include "InstructionBase.h"
#include "Instruction.h"
#include "macros.h"
#include <vector>


namespace ir {
class Graph;

class BasicBlock {
public:
    BasicBlock();
    BasicBlock(Graph *graph);
    NO_COPY_SEMANTIC(BasicBlock);
    NO_MOVE_SEMANTIC(BasicBlock);
    virtual DEFAULT_DTOR(BasicBlock);

    size_t GetId() const {
        return id;
    }
    std::vector<BasicBlock *> &GetPredecessors() {
        return preds;
    }
    const std::vector<BasicBlock *> &GetPredecessors() const {
        return preds;
    }
    std::vector<BasicBlock *> &GetSuccessors() {
        return succs;
    }
    const std::vector<BasicBlock *> &GetSuccessors() const {
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

    BasicBlock *GetDominator() {
        return dominator;
    }
    const BasicBlock *GetDominator() const {
        return dominator;
    }
    std::vector<BasicBlock *> &GetDominatedBlocks() {
        return dominated;
    }
    const std::vector<BasicBlock *> &GetDominatedBlocks() const {
        return dominated;
    }

    Graph *GetGraph() {
        return graph;
    }
    const Graph *GetGraph() const {
        return graph;
    }

    void SetId(size_t newId) {
        id = newId;
    }
    void AddPredecessor(BasicBlock *bblock);
    void AddSuccessor(BasicBlock *bblock);
    void RemovePredecessor(BasicBlock *bblock);
    void RemoveSuccessor(BasicBlock *bblock);

    void SetDominator(BasicBlock *newIDom) {
        dominator = newIDom;
    }
    void AddDominatedBlock(BasicBlock *bblock) {
        dominated.push_back(bblock);
    }

    void SetGraph(Graph *g) {
        graph = g;
    }

    void PushForwardInstruction(InstructionBase *instr);
    void PushBackInstruction(InstructionBase *instr);
    void InsertBefore(InstructionBase *before, InstructionBase *target);
    void InsertAfter(InstructionBase *after, InstructionBase *target);
    void UnlinkInstruction(InstructionBase *target);

    static constexpr size_t INVALID_ID = static_cast<size_t>(-1);

private:
    template <bool PushBack>
    void pushInstruction(InstructionBase *instr);

    void pushPhi(InstructionBase *instr);

private:
    size_t id;

    std::vector<BasicBlock *> preds;
    std::vector<BasicBlock *> succs;

    PhiInstruction *firstPhi;
    InstructionBase *firstInst;
    InstructionBase *lastInst;

    BasicBlock *dominator;
    std::vector<BasicBlock *> dominated;

    Graph *graph;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_
