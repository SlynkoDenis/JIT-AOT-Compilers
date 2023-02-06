#ifndef JIT_AOT_COMPILERS_COURSE_DCE_H_
#define JIT_AOT_COMPILERS_COURSE_DCE_H_

#include "dumper/DummyDumper.h"
#include "dumper/EventDumper.h"
#include "Graph.h"
#include "macros.h"


namespace ir {
class DCEPass {
public:
    explicit DCEPass(Graph *graph, bool shouldDump = true)
        : graph(graph),
          deadInstrs(graph->GetAllocator()->ToSTL())
    {
        ASSERT(graph);
        if (shouldDump) {
            dumper = utils::dumper::EventDumper::AddDumper(graph->GetAllocator(), PASS_NAME);
        } else {
            dumper = utils::dumper::EventDumper::AddDumper<utils::dumper::DummyDumper>(
                graph->GetAllocator(), utils::dumper::DummyDumper::DUMPER_NAME);
        }
    }
    NO_COPY_SEMANTIC(DCEPass);
    NO_MOVE_SEMANTIC(DCEPass);
    virtual DEFAULT_DTOR(DCEPass);

    void Run();

private:
    static bool instructionHasSideEffects(InstructionBase *instr);

    void markAlive(InstructionBase *instr);
    void markDead(InstructionBase *instr);
    void removeDead();

private:
    static constexpr const char *PASS_NAME = "dce";

private:
    Graph *graph;

    Marker aliveMarker;

    utils::memory::ArenaVector<InstructionBase *> deadInstrs;

    utils::dumper::EventDumper *dumper;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DCE_H_
