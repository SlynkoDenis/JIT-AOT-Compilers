#include "BasicBlock.h"


namespace ir {
bool BasicBlock::Dominates(const BasicBlock *bblock) const {
    ASSERT(bblock);
    auto *dom = bblock->GetDominator();
    while (dom != nullptr) {
        if (dom == this) {
            return true;
        }
        dom = dom->GetDominator();
    }
    return false;
}

void BasicBlock::AddPredecessor(BasicBlock *bblock) {
    ASSERT(bblock);
    ASSERT(std::find(preds.begin(), preds.end(), bblock) == preds.end());
    preds.push_back(bblock);
}

void BasicBlock::AddSuccessor(BasicBlock *bblock) {
    ASSERT(bblock);
    ASSERT(succs.size() < 2);
    ASSERT(std::find(succs.begin(), succs.end(), bblock) == succs.end());
    succs.push_back(bblock);
}

void BasicBlock::RemovePredecessor(BasicBlock *bblock) {
    ASSERT(bblock);
    auto it = std::find(preds.begin(), preds.end(), bblock);
    ASSERT(it != preds.end());

    *it = preds.back();
    preds.pop_back();
}

void BasicBlock::RemoveSuccessor(BasicBlock *bblock) {
    ASSERT(bblock);
    auto it = std::find(succs.begin(), succs.end(), bblock);
    ASSERT(it != succs.end());
    succs.erase(it);
}

void BasicBlock::ReplaceSuccessor(BasicBlock *prevSucc, BasicBlock *newSucc) {
    ASSERT((prevSucc) && (newSucc));
    auto it = std::find(succs.begin(), succs.end(), prevSucc);
    ASSERT(it != succs.end());
    *it = newSucc;
}

template <bool PushBack>
void BasicBlock::pushInstruction(InstructionBase *instr) {
    ASSERT((instr) && (instr->GetBasicBlock() == nullptr)
        && (instr->GetPrevInstruction() == nullptr));
    instr->SetBasicBlock(this);

    if (instr->IsPhi()) {
        pushPhi(instr);
    } else if (firstInst == nullptr) {
        instr->SetPrevInstruction(lastPhi);
        firstInst = instr;
        lastInst = instr;
        if (lastPhi) {
            lastPhi->SetNextInstruction(instr);
        }
    } else {
        if constexpr (PushBack) {
            instr->SetPrevInstruction(lastInst);
            lastInst->SetNextInstruction(instr);
            lastInst = instr;
        } else {
            instr->SetNextInstruction(firstInst);
            firstInst->SetPrevInstruction(instr);
            firstInst = instr;
        }
    }
    instrsCount += 1;
}

void BasicBlock::pushPhi(InstructionBase *instr) {
    ASSERT((instr) && instr->IsPhi());

    if (firstPhi == nullptr) {
        firstPhi = static_cast<PhiInstruction *>(instr);
        lastPhi = firstPhi;
        lastPhi->SetNextInstruction(firstInst);
        if (firstInst) {
            firstInst->SetPrevInstruction(lastPhi);
        }
    } else {
        instr->SetNextInstruction(firstPhi);
        firstPhi->SetPrevInstruction(instr);
        firstPhi = static_cast<PhiInstruction *>(instr);
    }
}

void BasicBlock::PushForwardInstruction(InstructionBase *instr) {
    pushInstruction<false>(instr);
}

void BasicBlock::PushBackInstruction(InstructionBase *instr) {
    pushInstruction<true>(instr);
}

void BasicBlock::InsertBefore(InstructionBase *before, InstructionBase *target) {
    // use PushBackInstruction/PushForwardInstruction for PHI instructions
    ASSERT((target) && (target->GetBasicBlock() == nullptr) && !target->IsPhi());
    ASSERT((before) && (before->GetBasicBlock() == this));
    target->SetBasicBlock(this);
    auto *prev = before->GetPrevInstruction();
    before->SetPrevInstruction(target);
    target->SetPrevInstruction(prev);
    target->SetNextInstruction(before);
    prev->SetNextInstruction(target);

    if (!prev) {
        firstInst = target;
    }
    instrsCount += 1;
}

void BasicBlock::InsertAfter(InstructionBase *after, InstructionBase *target) {
    ASSERT((target) && (target->GetBasicBlock() == nullptr) && !target->IsPhi());
    ASSERT((after) && (after->GetBasicBlock() == this));
    target->SetBasicBlock(this);
    auto *next = after->GetNextInstruction();
    after->SetNextInstruction(target);
    target->SetPrevInstruction(after);
    target->SetNextInstruction(next);
    next->SetPrevInstruction(target);

    if (!next) {
        lastInst = target;
    }
    instrsCount += 1;
}

// TODO: write unit test for this method
void BasicBlock::UnlinkInstruction(InstructionBase *target) {
    ASSERT((target) && (target->GetBasicBlock() == this));
    target->SetBasicBlock(nullptr);
    auto *prev = target->GetPrevInstruction();
    auto *next = target->GetNextInstruction();
    target->SetPrevInstruction(nullptr);
    target->SetNextInstruction(nullptr);

    // basic connection
    if (prev) {
        prev->SetNextInstruction(next);
    }
    if (next) {
        next->SetPrevInstruction(prev);
    }

    // update members of basic block
    if (target->IsPhi()) {
        if (target == firstPhi) {
            if (target == lastPhi) {
                firstPhi = nullptr;
                lastPhi = nullptr;
            } else {
                ASSERT((next) && next->IsPhi());
                firstPhi = static_cast<PhiInstruction *>(next);
            }
        } else if (target == lastPhi) {
            ASSERT((prev) && prev->IsPhi());
            lastPhi = static_cast<PhiInstruction *>(prev);
        }
    } else {
        if (target == firstInst) {
            firstInst = next;
        }
        if (target == lastInst) {
            lastInst = (prev && prev->IsPhi()) ? nullptr : prev;
        }
    }
    instrsCount -= 1;
}

void BasicBlock::ReplaceInstruction(InstructionBase *prevInstr, InstructionBase *newInstr) {
    prevInstr->ReplaceInputInUsers(newInstr);
    replaceInControlFlow(prevInstr, newInstr);
    // TODO: replace users in inputs of prevInstr to newInstr
}

void BasicBlock::replaceInControlFlow(InstructionBase *prevInstr, InstructionBase *newInstr) {
    // TODO: check case newInstr->IsPhi() -> true (with tests)
    ASSERT((prevInstr) && (prevInstr->GetBasicBlock() == this));
    if (prevInstr->GetPrevInstruction()) {
        InsertAfter(prevInstr->GetPrevInstruction(), newInstr);
    } else if (prevInstr->GetNextInstruction()) {
        InsertBefore(prevInstr->GetNextInstruction(), newInstr);
    } else {
        PushForwardInstruction(newInstr);
    }
    UnlinkInstruction(prevInstr);
}

// defined here after full declaration of BasicBlock's methods
BasicBlock *JumpInstruction::GetDestination() {
    auto *bblock = GetBasicBlock();
    ASSERT(bblock);
    auto successors = bblock->GetSuccessors();
    ASSERT(!successors.empty());
    return successors[0];
}

BasicBlock *CondJumpInstruction::GetTrueDestination() {
    return getBranchDestinationImpl<0>();
}

BasicBlock *CondJumpInstruction::GetFalseDestination() {
    return getBranchDestinationImpl<1>();
}

template <int CmpRes>
BasicBlock *CondJumpInstruction::getBranchDestinationImpl() {
    auto *bblock = GetBasicBlock();
    ASSERT(bblock);
    auto successors = bblock->GetSuccessors();
    ASSERT(successors.size() == 2);
    return successors[CmpRes];
}
}   // namespace ir
