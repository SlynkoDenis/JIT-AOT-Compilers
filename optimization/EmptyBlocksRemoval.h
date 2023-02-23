#ifndef JIT_AOT_COMPILERS_COURSE_EMPTY_BLOCKS_REMOVAL_H_
#define JIT_AOT_COMPILERS_COURSE_EMPTY_BLOCKS_REMOVAL_H_

#include "PassBase.h"


namespace ir {
class EmptyBlocksRemoval : public PassBase {
public:
    explicit EmptyBlocksRemoval(Graph *graph, bool shouldDump = false)
        : PassBase(graph, shouldDump) {}
    ~EmptyBlocksRemoval() noexcept override = default;

    void Run() override {
        graph->ForEachBasicBlock(RemoveIfEmpty);
    }

    const char *GetName() const override {
        return PASS_NAME;
    }

    static void RemoveIfEmpty(BasicBlock *bblock);

private:
    static constexpr const char *PASS_NAME = "empty_blocks_removal";
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_EMPTY_BLOCKS_REMOVAL_H_
