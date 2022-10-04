#ifndef JIT_AOT_COMPILERS_COURSE_IR_BUILDER_H_
#define JIT_AOT_COMPILERS_COURSE_IR_BUILDER_H_

#include "BasicBlock.h"
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

class IRBuilder : public IRBuilderBase {
public:
    IRBuilder() : IRBuilderBase(), graph(nullptr) {}
    virtual ~IRBuilder() noexcept {
        Clear();
    }

    void CreateGraph() {
        ASSERT(graph == nullptr);
        graph = new Graph();
    }

    BasicBlock *CreateEmptyBasicBlock() override {
        auto *bblock = new BasicBlock();
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
        for (auto *bblock : bblocks) {
            delete bblock;
        }
        bblocks.clear();
        delete graph;
        graph = nullptr;
    }

private:
    std::vector<BasicBlock *> bblocks;
    Graph *graph;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_IR_BUILDER_H_
