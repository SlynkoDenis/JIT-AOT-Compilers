#ifndef JIT_AOT_COMPILERS_COURSE_CONSTANT_FOLDING_H_
#define JIT_AOT_COMPILERS_COURSE_CONSTANT_FOLDING_H_

#include "arena/ArenaAllocator.h"
#include "BasicBlock.h"
#include "Graph.h"
#include "macros.h"


namespace ir {
class ConstantFolding {
public:
    ConstantFolding() = default;
    NO_COPY_SEMANTIC(ConstantFolding);
    NO_MOVE_SEMANTIC(ConstantFolding);
    virtual DEFAULT_DTOR(ConstantFolding);

    virtual bool ProcessAND(BinaryRegInstruction *instr);
    virtual bool ProcessSRA(BinaryRegInstruction *instr);
    virtual bool ProcessSUB(BinaryRegInstruction *instr);

private:
    static ConstantInstruction *asConst(InstructionBase *instr);
    static InstructionBuilder *getInstructionBuilder(InstructionBase *instr);
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_CONSTANT_FOLDING_H_