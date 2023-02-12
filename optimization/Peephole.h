#ifndef JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_
#define JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_

#include "arena/ArenaAllocator.h"
#include "ConstantFolding.h"
#include "dumper/DummyDumper.h"
#include "dumper/EventDumper.h"
#include "InstructionBase.h"
#include "Graph.h"
#include "macros.h"


namespace ir {
class PeepholePass {
public:
    explicit PeepholePass(Graph *graph, bool shouldDump = true) : graph(graph) {
        ASSERT(graph);
        if (shouldDump) {
            dumper = utils::dumper::EventDumper::AddDumper(graph->GetAllocator(), PASS_NAME);
        } else {
            dumper = utils::dumper::EventDumper::AddDumper<utils::dumper::DummyDumper>(
                graph->GetAllocator(), utils::dumper::DummyDumper::DUMPER_NAME);
        }
    }
    NO_COPY_SEMANTIC(PeepholePass);
    NO_MOVE_SEMANTIC(PeepholePass);
    virtual DEFAULT_DTOR(PeepholePass);

    void Run();

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
    Graph *graph;

    ConstantFolding foldingPass;

    utils::dumper::EventDumper *dumper;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_PEEPHOLE_H_
