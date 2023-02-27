#ifndef JIT_AOT_COMPILERS_COURSE_EMPTY_BLOCKS_REMOVAL_H_
#define JIT_AOT_COMPILERS_COURSE_EMPTY_BLOCKS_REMOVAL_H_

#include "logger.h"
#include "PassBase.h"


namespace ir {
class EmptyBlocksRemoval : public PassBase, public utils::Logger {
public:
    explicit EmptyBlocksRemoval(Graph *graph)
        : PassBase(graph), utils::Logger(log4cpp::Category::getInstance(GetName())) {}
    ~EmptyBlocksRemoval() noexcept override = default;

    void Run() override {
        bool wasRemoved = false;
        auto removeCallback = [&wasRemoved](BasicBlock *bblock) {
            RemoveIfEmpty(bblock, wasRemoved);
        };
        graph->ForEachBasicBlock(removeCallback);
        if (wasRemoved) {
            postRemoval();
        }
    }

    const char *GetName() const {
        return PASS_NAME;
    }

    static void RemoveIfEmpty(BasicBlock *bblock, bool &removed);

private:
    void postRemoval();

private:
    static constexpr const char *PASS_NAME = "empty_blocks_removal";
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_EMPTY_BLOCKS_REMOVAL_H_
