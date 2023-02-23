#ifndef JIT_AOT_COMPILERS_COURSE_DOM_TREE_H_
#define JIT_AOT_COMPILERS_COURSE_DOM_TREE_H_

#include "arena/ArenaAllocator.h"
#include "DSU.h"
#include "PassBase.h"
#include <vector>


namespace ir {
class DomTreeBuilder : public PassBase {
public:
    using VectorBBlocks = ArenaVector<BasicBlock *>;

    explicit DomTreeBuilder(Graph *graph) : PassBase(graph, false) {}
    NO_COPY_SEMANTIC(DomTreeBuilder);
    NO_MOVE_SEMANTIC(DomTreeBuilder);
    ~DomTreeBuilder() noexcept override = default;

    void Run();

    void SetGraph(Graph *newGraph) {
        ASSERT(newGraph);
        graph = newGraph;
    }

    const VectorBBlocks *GetImmediateDominators() const {
        return idoms;
    }

public:
    static constexpr AnalysisFlag SET_FLAG = AnalysisFlag::DOM_TREE;

private:
    DSU resetStructs();
    void dfsTraverse(BasicBlock *bblock);
    void computeSDoms(DSU &sdomsHelper);
    void computeIDoms();

    size_t getSize() const {
        return idoms->size();
    }

    BasicBlock *getImmDominator(size_t id) {
        return idoms->at(id);
    }
    void setImmDominator(size_t id, BasicBlock *bblock) {
        idoms->at(id) = bblock;
    }

    size_t getSemiDomNumber(BasicBlock *bblock) {
        ASSERT(bblock);
        return sdoms->at(bblock->GetId());
    }
    void setSemiDomNumber(BasicBlock *bblock, size_t visitNumber) {
        ASSERT(bblock);
        sdoms->at(bblock->GetId()) = visitNumber;
    }

    const VectorBBlocks &getSemiDoms(BasicBlock *bblock) {
        ASSERT(bblock);
        return sdomsSet->at(bblock->GetId());
    }
    void registerSemiDom(BasicBlock *bblock) {
        auto semiDomNumber = getSemiDomNumber(bblock);
        sdomsSet->at(getOrderedBlock(semiDomNumber)->GetId()).push_back(bblock);
    }

    BasicBlock *getLabel(BasicBlock *bblock) {
        ASSERT(bblock);
        return labels->at(bblock->GetId());
    }

    BasicBlock *getOrderedBlock(size_t visitNumber) {
        return orderedBBlocks->at(visitNumber);
    }
    void setOrderedBlock(size_t visitNumber, BasicBlock *bblock) {
        ASSERT(bblock);
        orderedBBlocks->at(visitNumber) = bblock;
    }

    BasicBlock *getBlockDFOParent(BasicBlock *bblock) {
        ASSERT(bblock);
        return bblocksParents->at(bblock->GetId());
    }
    void setBlockDFOParent(BasicBlock *child, BasicBlock *parent) {
        ASSERT(child);
        ASSERT(parent);
        bblocksParents->at(child->GetId()) = parent;
    }

private:
    int lastNumber = 0;

    VectorBBlocks *idoms = nullptr;

    // mapping: node-id -> DFO-number of node's semidominator
    ArenaVector<size_t> *sdoms = nullptr;
    // mapping: node-id -> nodes dominated by this node-id
    ArenaVector<VectorBBlocks> *sdomsSet = nullptr;

    // nodes with min semidominators within DFO tree ancestors
    VectorBBlocks *labels = nullptr;
    // DFO basic blocks
    VectorBBlocks *orderedBBlocks = nullptr;
    // basic blocks' parents in DFO tree
    VectorBBlocks *bblocksParents = nullptr;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DOM_TREE_H_
