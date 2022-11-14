#include "DomTree.h"
#include <numeric>
#include "Traversals.h"


namespace ir {
void DomTreeBuilder::Build(Graph *graph) {
    ASSERT(graph);
    if (graph->IsEmpty()) {
        return;
    }

    auto sdomsHelper = resetStructs(graph);

    dfsTraverse(graph->GetFirstBasicBlock());
    // check graph's connectivity
    ASSERT(lastNumber == static_cast<int>(graph->GetBasicBlocksCount()) - 1);
    computeSDoms(sdomsHelper);
    computeIDoms();
}

DSU DomTreeBuilder::resetStructs(Graph *graph) {
    lastNumber = -1;

    auto bblocksCount = graph->GetBasicBlocksCount();
    auto *allocator = graph->GetAllocator();
    if (sdoms == nullptr) {
        sdoms = allocator->template NewVector<size_t>(bblocksCount, BasicBlock::INVALID_ID);
        sdomsSet = allocator->template NewVector<VectorBBlocks>(bblocksCount,
                                                                VectorBBlocks(allocator->ToSTL()));
        idoms = allocator->template NewVector<BasicBlock *>(bblocksCount, nullptr);
        labels = allocator->template NewVector<BasicBlock *>(bblocksCount, nullptr);
        orderedBBlocks = allocator->template NewVector<BasicBlock *>(bblocksCount, nullptr);
        bblocksParents = allocator->template NewVector<BasicBlock *>(bblocksCount, nullptr);
    } else {
        sdoms->clear();
        sdomsSet->clear();
        idoms->clear();
        labels->clear();
        orderedBBlocks->clear();
        bblocksParents->clear();

        sdoms->resize(bblocksCount, BasicBlock::INVALID_ID);
        sdomsSet->resize(bblocksCount, VectorBBlocks(allocator->ToSTL()));
        idoms->resize(bblocksCount, nullptr);
        labels->resize(bblocksCount, nullptr);
        orderedBBlocks->resize(bblocksCount, nullptr);
        bblocksParents->resize(bblocksCount, nullptr);
    }

    return DSU(labels, sdoms, allocator);
}

void DomTreeBuilder::dfsTraverse(BasicBlock *bblock) {
    ++lastNumber;
    ASSERT((bblock) && (lastNumber < static_cast<int>(getSize())));

    auto id = bblock->GetId();
    labels->at(id) = bblock;
    setSemiDomNumber(bblock, lastNumber);
    setOrderedBlock(lastNumber, bblock);

    for (auto *succ : bblock->GetSuccessors()) {
        if (getLabel(succ) == nullptr) {
            setBlockDFOParent(succ, bblock);
            dfsTraverse(succ);
        }
    }
}

void DomTreeBuilder::computeSDoms(DSU &sdomsHelper) {
    for (int i = getSize() - 1; i >= 0; --i) {
        auto *currentBlock = getOrderedBlock(i);

        for (const auto &pred : currentBlock->GetPredecessors()) {
            auto nodeWithMinLabel = sdomsHelper.Find(pred);
            auto id = std::min(
                getSemiDomNumber(currentBlock), getSemiDomNumber(nodeWithMinLabel));
            setSemiDomNumber(currentBlock, id);
        }

        if (i > 0) {
            registerSemiDom(currentBlock);
            sdomsHelper.Unite(currentBlock, getBlockDFOParent(currentBlock));
        }

        for (auto dominatee : getSemiDoms(currentBlock)) {
            auto minSDom = sdomsHelper.Find(dominatee);

            auto dominateeId = dominatee->GetId();
            if (getSemiDomNumber(minSDom) == getSemiDomNumber(dominatee)) {
                setImmDominator(dominateeId, getOrderedBlock(getSemiDomNumber(dominatee)));
            } else {
                setImmDominator(dominateeId, minSDom);
            }
        }
    }
}

void DomTreeBuilder::computeIDoms() {
    for (size_t i = 1; i < getSize(); ++i) {
        auto *currentBlock = getOrderedBlock(i);
        auto currentBlockId = currentBlock->GetId();
        if (getImmDominator(currentBlockId) != getOrderedBlock(getSemiDomNumber(currentBlock))) {
            setImmDominator(currentBlockId,
                            getImmDominator(getImmDominator(currentBlockId)->GetId()));
        }

        auto *immDom = getImmDominator(currentBlockId);
        currentBlock->SetDominator(immDom);
        immDom->AddDominatedBlock(currentBlock);
    }
}
}   // namespace ir
