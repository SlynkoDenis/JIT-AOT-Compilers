#include "DomTree.h"
#include <numeric>
#include "Traversals.h"


namespace ir {
void DomTreeBuilder::Run() {
    if (graph->IsEmpty()) {
        return;
    }

    auto sdomsHelper = resetStructs();

    dfsTraverse(graph->GetFirstBasicBlock());
    // check graph's connectivity
    ASSERT(lastNumber == static_cast<int>(graph->GetBasicBlocksCount()) - 1);
    computeSDoms(sdomsHelper);
    computeIDoms();
}

DSU DomTreeBuilder::resetStructs() {
    lastNumber = -1;

    auto bblocksCount = graph->GetBasicBlocksCount();
    sdoms.resize(bblocksCount, BasicBlock::INVALID_ID);
    sdomsSet.resize(bblocksCount);
    idoms.resize(bblocksCount, nullptr);
    labels.resize(bblocksCount, nullptr);
    orderedBBlocks.resize(bblocksCount, nullptr);
    bblocksParents.resize(bblocksCount, nullptr);

    return DSU(labels, sdoms, graph->GetMemoryResource());
}

void DomTreeBuilder::dfsTraverse(BasicBlock *bblock) {
    ++lastNumber;
    ASSERT((bblock) && (lastNumber < static_cast<int>(getSize())));

    auto id = bblock->GetId();
    labels.at(id) = bblock;
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
