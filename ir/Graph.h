#ifndef JIT_AOT_COMPILERS_COURSE_GRAPH_H_
#define JIT_AOT_COMPILERS_COURSE_GRAPH_H_

#include "BasicBlock.h"
#include "macros.h"


namespace ir {
class Loop;

class Graph {
public:
    Graph() : firstBlock(nullptr), lastBlock(nullptr), loopTreeRoot(nullptr) {}
    NO_COPY_SEMANTIC(Graph);
    NO_MOVE_SEMANTIC(Graph);
    virtual DEFAULT_DTOR(Graph);

    BasicBlock *GetFirstBasicBlock() {
        return firstBlock;
    }
    const BasicBlock *GetFirstBasicBlock() const {
        return firstBlock;
    }
    BasicBlock *GetLastBasicBlock() {
        return lastBlock;
    }
    const BasicBlock *GetLastBasicBlock() const {
        return lastBlock;
    }
    size_t GetBasicBlocksCount() const {
        return bblocks.size();
    }
    bool IsEmpty() const {
        return bblocks.empty();
    }

    Loop *GetLoopTree() {
        return loopTreeRoot;
    }
    const Loop *GetLoopTree() const {
        return loopTreeRoot;
    }

    void SetFirstBasicBlock(BasicBlock *bblock) {
        firstBlock = bblock;
    }
    void SetLastBasicBlock(BasicBlock *bblock) {
        lastBlock = bblock;
    }
    void SetLoopTree(Loop *loop) {
        loopTreeRoot = loop;
    }

    void ConnectBasicBlocks(BasicBlock *lhs, BasicBlock *rhs);
    void AddBasicBlock(BasicBlock *bblock);
    void AddBasicBlockBefore(BasicBlock *before, BasicBlock *bblock);
    void UnlinkBasicBlock(BasicBlock *bblock);
    void RemoveUnlinkedBlocks();

protected:
    void removePredecessors(BasicBlock *bblock);
    void removeSuccessors(BasicBlock *bblock);
    void updatePhiInstructions();

private:
    BasicBlock *firstBlock;
    BasicBlock *lastBlock;
    std::vector<BasicBlock *> bblocks;

    Loop *loopTreeRoot;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_H_
