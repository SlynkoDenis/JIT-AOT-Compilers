#ifndef JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_
#define JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_

#include "Instruction.h"
#include "macros.h"
#include <vector>


namespace ir {
class Graph;

class BasicBlock {
public:
    BasicBlock();
    explicit BasicBlock(Graph *graph);
    NO_COPY_SEMANTIC(BasicBlock);
    NO_MOVE_SEMANTIC(BasicBlock);
    virtual DEFAULT_DTOR(BasicBlock);

    size_t GetId() const {
        return id;
    }
    std::vector<BasicBlock *> GetPredecessors() const {
        return preds;
    }
    std::vector<BasicBlock *> GetSuccessors() const {
        return succs;
    }
    InstructionBase *GetFirstInstruction() const {
        return firstInst;
    }
    InstructionBase *GetLastInstruction() const {
        return lastInst;
    }
    InstructionBase *GetFirstPhiInstruction() const {
        return firstPhi;
    }
    Graph *GetGraph() const {
        return graph;
    }

    void SetId(size_t newId) {
        id = newId;
    }
    void AddPredecessor(BasicBlock *bblock);
    void AddSuccessor(BasicBlock *bblock);
    void SetGraph(Graph *g) {
        graph = g;
    }

    void PushForwardInstruction(InstructionBase *instr);
    void PushBackInstruction(InstructionBase *instr);
    void InsertBefore(InstructionBase *before, InstructionBase *target);
    void InsertAfter(InstructionBase *after, InstructionBase *target);
    void UnlinkInstruction(InstructionBase *target);

    static const size_t INVALID_ID = static_cast<size_t>(-1);

private:
    template <bool PushBack>
    void pushInstruction(InstructionBase *instr);

private:
    size_t id;

    std::vector<BasicBlock *> preds;
    std::vector<BasicBlock *> succs;
    
    InstructionBase *firstPhi;
    InstructionBase *firstInst;
    InstructionBase *lastInst;

    Graph *graph;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_BASIC_BLOCK_H_
