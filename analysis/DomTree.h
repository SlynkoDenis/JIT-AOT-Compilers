#ifndef JIT_AOT_COMPILERS_COURSE_DOM_TREE_H_
#define JIT_AOT_COMPILERS_COURSE_DOM_TREE_H_

#include "DSU.h"
#include "Graph.h"
#include <vector>


namespace ir {
class DomTreeBuilder final {
public:
    using VectorBBlocks = std::vector<BasicBlock *>;

    void Build(Graph *graph);

    const VectorBBlocks &GetImmediateDominators() const {
        return idoms;
    }

private:
    void resetStructs(size_t bblocksCount);
    void dfsTraverse(BasicBlock *bblock);
    void computeSDoms();
    void computeIDoms();

    int lastNumber;
    DSU sdomsHelper;

    VectorBBlocks idoms;

    // mapping: node-id -> DFO-number of node's semidominator
    std::vector<size_t> sdoms;
    // mapping: node-id -> nodes dominated by this node-id
    std::vector<VectorBBlocks> sdomsSet;

    // nodes with min semidominators within DFO tree ancestors
    VectorBBlocks labels;
    // DFO basic blocks
    VectorBBlocks orderedBBlocks;
    // basic blocks' parents in DFO tree
    VectorBBlocks bblocksParents;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DOM_TREE_H_
