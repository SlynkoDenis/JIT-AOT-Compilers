#include "InstructionBuilder.h"


namespace ir {
// implement Copy overrided methods for general classes
#define OVERRIDE_GENERAL_CLASS_COPY(name, ...)                                      \
name *name::Copy(BasicBlock *targetBBlock) const {                                  \
    auto *allocator = targetBBlock->GetGraph()->GetAllocator();                     \
    auto *instr = allocator->template New<name>(__VA_ARGS__, allocator);            \
    targetBBlock->GetGraph()->GetInstructionBuilder()->AttachInstruction(instr);    \
    instr->SetProperty(GetProperties());                                            \
    return instr;                                                                   \
}

OVERRIDE_GENERAL_CLASS_COPY(BinaryImmInstruction, GetOpcode(), GetType(), GetInput(0), GetValue())
OVERRIDE_GENERAL_CLASS_COPY(BinaryRegInstruction, GetOpcode(), GetType(), GetInput(0), GetInput(1))
OVERRIDE_GENERAL_CLASS_COPY(UnaryRegInstruction, GetOpcode(), GetType(), GetInput(0))

#undef OVERRIDE_GENERAL_CLASS_COPY

// special instructions' classes implementations
#define OVERRIDE_COPY_METHOD_FIXED(name, opcode)                            \
name *name::Copy(BasicBlock *targetBBlock) const {                          \
    auto *instrBuilder = targetBBlock->GetGraph()->GetInstructionBuilder(); \
    return instrBuilder->Create##opcode ();                                 \
}

#define OVERRIDE_COPY_METHOD(name, opcode, ...)                             \
name *name::Copy(BasicBlock *targetBBlock) const {                          \
    auto *instrBuilder = targetBBlock->GetGraph()->GetInstructionBuilder(); \
    return instrBuilder->Create##opcode (__VA_ARGS__);                      \
}

OVERRIDE_COPY_METHOD(ConstantInstruction, CONST, GetType(), GetValue())
OVERRIDE_COPY_METHOD(CastInstruction, CAST, GetType(), GetTargetType(), GetInput(0))
OVERRIDE_COPY_METHOD(CompareInstruction, CMP, GetType(), GetCondCode(), GetInput(0), GetInput(1))
OVERRIDE_COPY_METHOD_FIXED(CondJumpInstruction, JCMP)
OVERRIDE_COPY_METHOD_FIXED(JumpInstruction, JMP)
OVERRIDE_COPY_METHOD(RetInstruction, RET, GetType(), GetInput(0))
OVERRIDE_COPY_METHOD_FIXED(RetVoidInstruction, RETVOID)
OVERRIDE_COPY_METHOD(CallInstruction, CALL, GetType(), GetCallTarget(), GetInputs())
OVERRIDE_COPY_METHOD(LoadInstruction, LOAD, GetType(), GetValue())
OVERRIDE_COPY_METHOD(StoreInstruction, STORE, GetInput(0), GetValue())
OVERRIDE_COPY_METHOD(PhiInstruction, PHI, GetType(), GetInputs(), GetSourceBasicBlocks())
OVERRIDE_COPY_METHOD(InputArgumentInstruction, ARG, GetType())

#undef OVERRIDE_COPY_METHOD_FIXED
#undef OVERRIDE_COPY_METHOD
}   // namespace ir
