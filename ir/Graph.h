#ifndef JIT_AOT_COMPILERS_COURSE_GRAPH_H_
#define JIT_AOT_COMPILERS_COURSE_GRAPH_H_

#include "BasicBlock.h"
#include "macros.h"


namespace ir {
class Graph {
public:
    Graph() : firstBlock(nullptr), lastBlock(nullptr) {}
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

    void SetFirstBasicBlock(BasicBlock *bblock) {
        firstBlock = bblock;
    }
    void SetLastBasicBlock(BasicBlock *bblock) {
        lastBlock = bblock;
    }

    void AddBasicBlock(BasicBlock *bblock);
    void AddBasicBlockBefore(BasicBlock *before, BasicBlock *bblock);
    void AddBasicBlockAfter(BasicBlock *after, BasicBlock *bblock);
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
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_H_
