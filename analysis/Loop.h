#ifndef JIT_AOT_COMPILERS_COURSE_LOOP_H_
#define JIT_AOT_COMPILERS_COURSE_LOOP_H_

#include "BasicBlock.h"
#include "macros.h"
#include <vector>


namespace ir {
enum class DFSColors : uint32_t {
    WHITE = 0,
    GREY,
    BLACK,
    COLORS_SIZE = BLACK
};

class Loop final {
public:
    Loop(size_t id, BasicBlock *header, bool isIrreducible, bool isRoot = false)
        : id(id),
          header(header),
          outerLoop(nullptr),
          isIrreducible(isIrreducible),
          isRoot(isRoot) {}

    auto GetId() const {
        return id;
    }

    BasicBlock *GetHeader() {
        return header;
    }
    const BasicBlock *GetHeader() const {
        return header;
    }

    void AddBackEdge(BasicBlock *backEdgeSource) {
        ASSERT(std::find(backEdges.begin(), backEdges.end(), backEdgeSource) == backEdges.end());
        backEdges.push_back(backEdgeSource);
    }
    std::vector<BasicBlock *> GetBackEdges() {
        return backEdges;
    }
    const std::vector<BasicBlock *> &GetBackEdges() const {
        return backEdges;
    }

    std::vector<BasicBlock *> GetBasicBlocks() {
        return basicBlocks;
    }
    const std::vector<BasicBlock *> &GetBasicBlocks() const {
        return basicBlocks;
    }
    void AddBasicBlock(BasicBlock *bblock) {
        ASSERT((bblock)
               && (bblock->GetLoop() == nullptr)
               && (std::find(basicBlocks.begin(), basicBlocks.end(), bblock) == basicBlocks.end()));
        bblock->SetLoop(this);
        basicBlocks.push_back(bblock);
    }

    Loop *GetOuterLoop() {
        return outerLoop;
    }
    const Loop *GetOuterLoop() const {
        return outerLoop;
    }
    void SetOuterLoop(Loop *loop) {
        outerLoop = loop;
    }
    const std::vector<Loop *> &GetInnerLoops() const {
        return innerLoops;
    }
    void AddInnerLoop(Loop *loop) {
        ASSERT(std::find(innerLoops.begin(), innerLoops.end(), loop) == innerLoops.end());
        innerLoops.push_back(loop);
    }

    void SetIrreducibility(bool isIrr) {
        isIrreducible = isIrr;
    }
    bool IsIrreducible() const {
        return isIrreducible;
    }

    bool IsRoot() const {
        return isRoot;
    }

private:
    size_t id;

    BasicBlock *header;
    // TODO: may replace with set
    std::vector<BasicBlock *> backEdges;

    std::vector<BasicBlock *> basicBlocks;

    Loop *outerLoop;
    std::vector<Loop *> innerLoops;

    bool isIrreducible;

    // TODO: can replace with `outerLoop == nullptr`
    bool isRoot;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LOOP_H_
