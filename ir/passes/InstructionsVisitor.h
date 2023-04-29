#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTIONS_VISITOR_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTIONS_VISITOR_H_

#include <array>
#include "BasicBlock.h"


namespace ir {
template <typename T>
class InstructionsVisitor {
public:
    InstructionsVisitor() = default;
    DEFAULT_COPY_SEMANTIC(InstructionsVisitor);
    DEFAULT_MOVE_SEMANTIC(InstructionsVisitor);
    virtual DEFAULT_DTOR(InstructionsVisitor);

    void Visit(BasicBlock *bblock);
};

template <typename T>
void InstructionsVisitor<T>::Visit(BasicBlock *bblock) {
#define DEF(name) &&HANDLE_##name,
    static std::array<const void*, static_cast<size_t>(Opcode::NUM_OPCODES) + 1> dispatchTable {
        INSTS_LIST(DEF)
        &&HANDLE_INVALID
    };
#undef DEF

    ASSERT(bblock);
    auto *instr = bblock->GetStartingInstruction();
    if (!instr) {
        return;
    }
    goto *dispatchTable[static_cast<size_t>(instr->GetOpcode())];

#define DEF(name)                                                           \
HANDLE_##name:  {                                                           \
    constexpr bool hasVisitor##name = requires(T &t, InstructionBase *i) {  \
        t.visit##name(i);                                                   \
    };                                                                      \
    if constexpr (hasVisitor##name) {                                       \
        static_cast<T *>(this)->visit##name(instr);                         \
    }                                                                       \
    instr = instr->GetNextInstruction();                                    \
    if (!instr) {                                                           \
        return;                                                             \
    }                                                                       \
    goto *dispatchTable[static_cast<size_t>(instr->GetOpcode())];           \
}

INSTS_LIST(DEF)

#undef DEF

HANDLE_INVALID: {
    UNREACHABLE("invalid instruction detected");
}
}
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTIONS_VISITOR_H_
