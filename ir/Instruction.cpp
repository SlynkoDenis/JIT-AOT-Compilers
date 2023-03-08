#include "BasicBlock.h"
#include "instructions/Instruction.h"


namespace ir {
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
