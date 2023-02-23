#ifndef JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_
#define JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_

#include "arena/ArenaAllocator.h"
#include "ConstantFolding.h"
#include "Graph.h"
#include "PassBase.h"


namespace ir {
class PeepholePass : public PassBase {
public:
    explicit PeepholePass(Graph *graph, bool shouldDump = false)
        : PassBase(graph, shouldDump)
    {}
    ~PeepholePass() noexcept override = default;

    void Run() override;

    const char *GetName() const override {
        return PASS_NAME;
    }

    void ProcessAND(InstructionBase *instr);
    void ProcessSRA(InstructionBase *instr);
    void ProcessSUB(InstructionBase *instr);

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
    ConstantFolding foldingPass;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_
