#include <array>
#include "BasicBlock.h"
#include "InstructionBase.h"


namespace ir {
const char *getOpcodeName(Opcode opcode) {
    static constexpr std::array<const char *, static_cast<size_t>(Opcode::NUM_OPCODES)> names{
#define OPCODE_NAME(name, ...) #name,
    INSTS_LIST(OPCODE_NAME)
#undef OPCODE_NAME
    };
    return names[static_cast<size_t>(opcode)];
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
}   // namespace ir
