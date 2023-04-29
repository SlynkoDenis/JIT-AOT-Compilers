#ifndef JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_
#define JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_

#include "AllocatorUtils.h"
#include "Graph.h"
#include "logger.h"
#include "passes/InstructionsVisitor.h"
#include "passes/PassBase.h"


namespace ir {
class PeepholePass : public PassBase, public utils::Logger, public InstructionsVisitor<PeepholePass> {
public:
    explicit PeepholePass(Graph *graph)
        : PassBase(graph), utils::Logger(log4cpp::Category::getInstance(GetName())) {}
    ~PeepholePass() noexcept override = default;

    bool Run() override;

    const char *GetName() const {
        return PASS_NAME;
    }

    void visitAND(InstructionBase *instr);
    void visitSRA(InstructionBase *instr);
    void visitSUB(InstructionBase *instr);

private:
    bool tryConstantAND(BinaryRegInstruction *instr, Input checked, Input second);
    bool tryANDAfterNOT(BinaryRegInstruction *instr);
    bool tryANDRepeatedArgs(BinaryRegInstruction *instr);
    bool trySequencedSRA(BinaryRegInstruction *instr);
    bool trySRAZero(BinaryRegInstruction *instr);
    bool trySUBAfterNEG(BinaryRegInstruction *instr, Input checked, Input second);
    bool trySUBAfterADD(BinaryRegInstruction *instr);
    bool trySUBZero(BinaryRegInstruction *instr);
    bool trySUBRepeatedArgs(BinaryRegInstruction *instr);

    void replaceWithoutNewInstr(BinaryRegInstruction *instr, InstructionBase *replacedInstr);

private:
    static constexpr const char *PASS_NAME = "peephole";

private:
    bool applied = false;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_
