#ifndef JIT_AOT_COMPILERS_COURSE_CONSTANT_FOLDING_H_
#define JIT_AOT_COMPILERS_COURSE_CONSTANT_FOLDING_H_

#include "AllocatorUtils.h"
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

    static bool ProcessAND(BinaryRegInstruction *instr);
    static bool ProcessSRA(BinaryRegInstruction *instr);
    static bool ProcessSUB(BinaryRegInstruction *instr);

    static void ReplaceWithConst(InstructionBase *instr, ConstantInstruction *targetConst);

private:
    static InstructionBuilder *getInstructionBuilder(InstructionBase *instr);
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_CONSTANT_FOLDING_H_
