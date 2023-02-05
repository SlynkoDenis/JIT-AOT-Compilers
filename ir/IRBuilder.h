#ifndef JIT_AOT_COMPILERS_COURSE_IR_BUILDER_H_
#define JIT_AOT_COMPILERS_COURSE_IR_BUILDER_H_

#include "arena/ArenaAllocator.h"
#include "BasicBlock.h"
#include "Graph.h"
#include "macros.h"


namespace ir {
class IRBuilderBase {
public:
    IRBuilderBase() = default;
    NO_COPY_SEMANTIC(IRBuilderBase);
    NO_MOVE_SEMANTIC(IRBuilderBase);
    virtual DEFAULT_DTOR(IRBuilderBase);

    virtual BasicBlock *CreateEmptyBasicBlock() = 0;
    virtual Graph *GetGraph() = 0;
    virtual const Graph *GetGraph() const = 0;
    virtual void Clear() noexcept = 0;
};

class IRBuilder final : public IRBuilderBase {
public:
    IRBuilder() = delete;
    IRBuilder(ArenaAllocator *const alloc)
        : IRBuilderBase(),
          allocator(alloc),
          bblocks(alloc->ToSTL())
    {
        ASSERT(allocator);
    }
    virtual ~IRBuilder() noexcept {
        Clear();
    }

    void CreateGraph() {
        ASSERT(graph == nullptr);
        auto *instrBuilder = allocator->template New<InstructionBuilder>(allocator);
        graph = allocator->template New<Graph>(allocator, instrBuilder);
    }

    BasicBlock *CreateEmptyBasicBlock() override {
        auto *bblock = allocator->template New<BasicBlock>(graph);
        bblocks.push_back(bblock);
        graph->AddBasicBlock(bblock);
        return bblock;
    }

    Graph *GetGraph() override {
        return graph;
    }
    const Graph *GetGraph() const override {
        return graph;
    }

    void Clear() noexcept override {
        graph = nullptr;
    }

private:
    ArenaAllocator *allocator = nullptr;

    utils::memory::ArenaVector<BasicBlock *> bblocks;
    Graph *graph = nullptr;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_IR_BUILDER_H_
