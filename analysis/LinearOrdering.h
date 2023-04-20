#ifndef JIT_AOT_COMPILERS_COURSE_LINEAR_ORDERING_H_
#define JIT_AOT_COMPILERS_COURSE_LINEAR_ORDERING_H_

#include <list>
#include "PassBase.h"


namespace ir {
class LinearOrdering final : public PassBase {
public:
    explicit LinearOrdering(Graph *graph) : PassBase(graph) {}

    bool Run() override;

private:
    void orderBlocks(std::pmr::vector<BasicBlock *> &newOrder);

    void queueCond(std::pmr::list<BasicBlock *> &remainedBlocks, BasicBlock *bblock);
    void addIntoQueue(std::pmr::list<BasicBlock *> &remainedBlocks, BasicBlock *bblock);

    bool mustInverseCondition(BasicBlock *bblock);
    bool unvisitedForwardEdgesExist(BasicBlock *bblock);

    BasicBlock *insertJmpBlock(BasicBlock *pred, BasicBlock *succ);

    void setNewOrder(std::pmr::vector<BasicBlock *> &&newOrder);
    void postOrder();

    bool isVisited(const BasicBlock *bblock) const {
        ASSERT(bblock);
        return bblock->IsMarkerSet(visitedMarker);
    }

private:
    Marker visitedMarker = utils::to_underlying(MarkersConstants::UNDEF_VALUE);
    bool cfgChanged = false;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_LINEAR_ORDERING_H_
