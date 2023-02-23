#include <array>
#include "BasicBlock.h"


namespace ir {
void InstructionBase::UnlinkFromParent() {
    ASSERT(parent);
    parent->UnlinkInstruction(this);
}

void InstructionBase::InsertBefore(InstructionBase *inst) {
    ASSERT(parent);
    parent->InsertBefore(inst, this);
}

void InstructionBase::InsertAfter(InstructionBase *inst) {
    ASSERT(parent);
    parent->InsertAfter(inst, this);
}

void InstructionBase::ReplaceInputInUsers(InstructionBase *newInput) {
    newInput->AddUsers(GetUsers());
    for (auto &it : GetUsers()) {
        ASSERT(it->HasInputs());
        auto *typed = static_cast<InputsInstruction *>(it);
        typed->ReplaceInput(this, newInput);
    }
}
}   // namespace ir
