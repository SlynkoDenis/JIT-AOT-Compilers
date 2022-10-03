#include "BasicBlock.h"


namespace ir {
BasicBlock::BasicBlock()
    : id(INVALID_ID),
      firstPhi(nullptr),
      firstInst(nullptr),
      lastInst(nullptr),
      graph(nullptr) {}

BasicBlock::BasicBlock(Graph *graph)
    : id(INVALID_ID),
      firstPhi(nullptr),
      firstInst(nullptr),
      lastInst(nullptr),
      graph(graph) {}

void BasicBlock::AddPredecessor(BasicBlock *bblock) {
    ASSERT(bblock);
    preds.push_back(bblock);
}

void BasicBlock::AddSuccessor(BasicBlock *bblock) {
    ASSERT(bblock);
    ASSERT(succs.size() < 2);
    succs.push_back(bblock);
}

template <bool PushBack>
void BasicBlock::pushInstruction(InstructionBase *instr) {
    ASSERT((instr) && (instr->GetBasicBlock() == nullptr)
        && (instr->GetPrevInstruction() == nullptr));
    instr->SetBasicBlock(this);
    if (firstInst == nullptr) {
        firstInst = instr;
        lastInst = instr;
    } else {
        if constexpr (PushBack) {
            instr->SetPrevInstruction(lastInst);
            lastInst = instr;
        } else {
            instr->SetNextInstruction(firstInst);
            firstInst = instr;
        }
    }
}

void BasicBlock::PushForwardInstruction(InstructionBase *instr) {
    pushInstruction<false>(instr);
}

void BasicBlock::PushBackInstruction(InstructionBase *instr) {
    pushInstruction<true>(instr);
}

void BasicBlock::InsertBefore(InstructionBase *before, InstructionBase *target) {
    ASSERT((target) && (target->GetBasicBlock() == nullptr));
    ASSERT((before) && (before->GetBasicBlock() == this));
    target->SetBasicBlock(this);
    auto *prev = before->GetPrevInstruction();
    before->SetPrevInstruction(target);
    target->SetPrevInstruction(prev);
    target->SetNextInstruction(before);

    if (!prev) {
        firstInst = target;
    }
}

void BasicBlock::InsertAfter(InstructionBase *after, InstructionBase *target) {
    ASSERT((target) && (target->GetBasicBlock() == nullptr));
    ASSERT((after) && (after->GetBasicBlock() == this));
    target->SetBasicBlock(this);
    auto *next = after->GetPrevInstruction();
    after->SetNextInstruction(target);
    target->SetPrevInstruction(after);
    target->SetNextInstruction(next);

    if (!next) {
        lastInst = target;
    }
}

void BasicBlock::UnlinkInstruction(InstructionBase *target) {
    ASSERT((target) && (target->GetBasicBlock() == this));
    target->SetBasicBlock(nullptr);
    auto *prev = target->GetPrevInstruction();
    auto *next = target->GetNextInstruction();
    target->SetPrevInstruction(nullptr);
    target->SetNextInstruction(nullptr);

    if (!prev) {
        firstInst = next;
    }
    if (!next) {
        lastInst = prev;
    }
}
}   // namespace
