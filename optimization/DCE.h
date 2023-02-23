#ifndef JIT_AOT_COMPILERS_COURSE_DCE_H_
#define JIT_AOT_COMPILERS_COURSE_DCE_H_

#include "Graph.h"
#include "PassBase.h"


namespace ir {
class DCEPass : public PassBase {
public:
    explicit DCEPass(Graph *graph, bool shouldDump = false)
        : PassBase(graph, shouldDump),
          deadInstrs(graph->GetAllocator()->ToSTL())
    {}
    ~DCEPass() noexcept override = default;

    void Run() override;

    const char *GetName() const override {
        return PASS_NAME;
    }

private:
    void markAlive(InstructionBase *instr);
    void markDead(InstructionBase *instr);
    void removeDead();

private:
    static constexpr const char *PASS_NAME = "dce";

private:
    Marker aliveMarker;

    utils::memory::ArenaVector<InstructionBase *> deadInstrs;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DCE_H_
