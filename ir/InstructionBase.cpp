#include <array>
#include "BasicBlock.h"


namespace ir {
Input InstructionBase::ToInput() {
    return {this};
}

ConstantInstruction *InstructionBase::AsConst() {
    ASSERT(IsConst());
    return static_cast<ConstantInstruction *>(this);
}

const ConstantInstruction *InstructionBase::AsConst() const {
    ASSERT(IsConst());
    return static_cast<const ConstantInstruction *>(this);
}

PhiInstruction *InstructionBase::AsPhi() {
    ASSERT(IsPhi());
    return static_cast<PhiInstruction *>(this);
}

const PhiInstruction *InstructionBase::AsPhi() const {
    ASSERT(IsPhi());
    return static_cast<const PhiInstruction *>(this);
}

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
