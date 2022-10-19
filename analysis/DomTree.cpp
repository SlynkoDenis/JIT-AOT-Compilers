#include "DomTree.h"
#include <numeric>
#include "Traversals.h"


namespace ir {
void DomTreeBuilder::Build(Graph *graph) {
    ASSERT(graph);
    if (graph->IsEmpty()) {
        return;
    }

    resetStructs(graph->GetBasicBlocksCount());

    dfsTraverse(graph->GetFirstBasicBlock());
    // check graph's connectivity
    ASSERT(lastNumber == static_cast<int>(graph->GetBasicBlocksCount()) - 1);
    computeSDoms();
    computeIDoms();
}

void DomTreeBuilder::resetStructs(size_t bblocksCount) {
    lastNumber = -1;
    sdoms.clear();
    sdoms.resize(bblocksCount, BasicBlock::INVALID_ID);
    sdomsSet.clear();
    sdomsSet.resize(bblocksCount);
    idoms.clear();
    idoms.resize(bblocksCount, nullptr);

    labels.clear();
    labels.resize(bblocksCount, nullptr);
    orderedBBlocks.clear();
    orderedBBlocks.resize(bblocksCount, nullptr);
    bblocksParents.clear();
    bblocksParents.resize(bblocksCount, nullptr);

    sdomsHelper = DSU(labels, sdoms);
}

void DomTreeBuilder::dfsTraverse(BasicBlock *bblock) {
    ++lastNumber;
    ASSERT((bblock) && (lastNumber < static_cast<int>(orderedBBlocks.size())));

    auto id = bblock->GetId();
    labels[id] = bblock;
    sdoms[id] = lastNumber;
    orderedBBlocks[lastNumber] = bblock;

    for (auto *succ : bblock->GetSuccessors()) {
        if (labels[succ->GetId()] == nullptr) {
            bblocksParents[succ->GetId()] = bblock;
            dfsTraverse(succ);
        }
    }
}

void DomTreeBuilder::computeSDoms() {
    for (int i = orderedBBlocks.size() - 1; i >= 0; --i) {
        auto *currentBlock = orderedBBlocks[i];
        auto currentBlockId = currentBlock->GetId();

        for (const auto &pred : currentBlock->GetPredecessors()) {
            auto nodeWithMinLabel = sdomsHelper.Find(pred);
            sdoms[currentBlockId] = std::min(
                sdoms[currentBlockId], sdoms[nodeWithMinLabel->GetId()]);
        }

        if (i > 0) {
            sdomsSet[orderedBBlocks[sdoms[currentBlockId]]->GetId()].push_back(currentBlock);
            auto *parent = bblocksParents[currentBlockId];
            sdomsHelper.Unite(currentBlock, parent);
        }

        for (auto dominatee : sdomsSet[currentBlockId]) {
            auto minSDom = sdomsHelper.Find(dominatee);

            auto dominateeId = dominatee->GetId();
            if (sdoms[minSDom->GetId()] == sdoms[dominateeId]) {
                idoms[dominateeId] = orderedBBlocks[sdoms[dominateeId]];
            } else {
                idoms[dominateeId] = minSDom;
            }
        }
    }
}

void DomTreeBuilder::computeIDoms() {
    for (size_t i = 1; i < orderedBBlocks.size(); ++i) {
        auto *currentBlock = orderedBBlocks[i];
        auto currentBlockId = currentBlock->GetId();
        if (idoms[currentBlockId]->GetId() != orderedBBlocks[sdoms[currentBlockId]]->GetId()) {
            idoms[currentBlockId] = idoms[idoms[currentBlockId]->GetId()];
        }

        currentBlock->SetDominator(idoms[currentBlockId]);
        idoms[currentBlockId]->AddDominatedBlock(currentBlock);
    }
}
}   // namespace ir
