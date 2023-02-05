#ifndef JIT_AOT_COMPILERS_COURSE_GRAPH_H_
#define JIT_AOT_COMPILERS_COURSE_GRAPH_H_

#include "arena/ArenaAllocator.h"
#include "BasicBlock.h"
#include "macros.h"


namespace ir {
class InstructionBuilder;
class Loop;

class Graph {
public:
    Graph(ArenaAllocator *allocator, InstructionBuilder* instrBuilder)
        : allocator(allocator),
          firstBlock(nullptr),
          lastBlock(nullptr),
          bblocks(allocator->ToSTL()),
          loopTreeRoot(nullptr),
          instrBuilder(instrBuilder) {}
    NO_COPY_SEMANTIC(Graph);
    NO_MOVE_SEMANTIC(Graph);
    virtual DEFAULT_DTOR(Graph);

    ArenaAllocator *GetAllocator() const {
        return allocator;
    }

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

    InstructionBuilder *GetInstructionBuilder() {
        return instrBuilder;
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
    ArenaAllocator *allocator;

    BasicBlock *firstBlock;
    BasicBlock *lastBlock;
    utils::memory::ArenaVector<BasicBlock *> bblocks;

    Loop *loopTreeRoot;

    InstructionBuilder *instrBuilder;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_H_
