#ifndef JIT_AOT_COMPILERS_COURSE_DCE_H_
#define JIT_AOT_COMPILERS_COURSE_DCE_H_

#include "dumper/DummyDumper.h"
#include "dumper/EventDumper.h"
#include "Graph.h"
#include "PassBase.h"


namespace ir {
class DCEPass : public OptimizationPassBase {
public:
    explicit DCEPass(Graph *graph, bool shouldDump = true)
        : OptimizationPassBase(graph),
          deadInstrs(graph->GetAllocator()->ToSTL())
    {
        if (shouldDump) {
            dumper = utils::dumper::EventDumper::AddDumper(graph->GetAllocator(), PASS_NAME).second;
        } else {
            dumper = utils::dumper::EventDumper::AddDumper<utils::dumper::DummyDumper>(
                graph->GetAllocator(), utils::dumper::DummyDumper::DUMPER_NAME).second;
        }
    }
    ~DCEPass() noexcept override = default;

    void Run() override;

private:
    static bool instructionHasSideEffects(InstructionBase *instr);

    void markAlive(InstructionBase *instr);
    void markDead(InstructionBase *instr);
    void removeDead();

private:
    static constexpr const char *PASS_NAME = "dce";

private:
    Marker aliveMarker;

    utils::memory::ArenaVector<InstructionBase *> deadInstrs;

    utils::dumper::EventDumper *dumper;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DCE_H_
