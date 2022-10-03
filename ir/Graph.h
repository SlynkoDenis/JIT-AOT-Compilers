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

    void AddBasicBlock(BasicBlock *bblock);
    void AddAsPredecessor(BasicBlock *newSuccessor, BasicBlock *bblock);
    void AddAsSuccessor(BasicBlock *newPredecessor, BasicBlock *bblock);
    void UnlinkBasicBlock(BasicBlock *bblock);
    void RemoveUnlinkedBlocks();

protected:
    void removePredecessors(BasicBlock *bblock);
    void removeSuccessors(BasicBlock *bblock);

private:
    BasicBlock *firstBlock;
    BasicBlock *lastBlock;
    std::vector<BasicBlock *> bblocks;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_H_
