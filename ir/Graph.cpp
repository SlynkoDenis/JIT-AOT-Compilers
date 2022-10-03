#include <algorithm>
#include "Graph.h"


namespace ir {
void Graph::AddBasicBlock(BasicBlock *bblock) {
    ASSERT(bblock);
    if (bblocks.empty()) {
        firstBlock = bblock;
        lastBlock = bblock;
    }
    bblock->SetId(bblocks.size());
    bblocks.push_back(bblock);
    bblock->SetGraph(this);
}

void Graph::AddAsPredecessor(BasicBlock *newSuccessor, BasicBlock *bblock) {
    ASSERT((newSuccessor) && (newSuccessor->GetGraph() == this));
    ASSERT((bblock) && (bblock->GetGraph() == nullptr));
    bblock->SetGraph(this);
    bblock->AddSuccessor(newSuccessor);
    newSuccessor->AddPredecessor(bblock);
    if (firstBlock == newSuccessor) {
        firstBlock = bblock;
    }
}

void Graph::AddAsSuccessor(BasicBlock *newPredecessor, BasicBlock *bblock) {
    ASSERT((newPredecessor) && (newPredecessor->GetGraph() == this));
    ASSERT((bblock) && (bblock->GetGraph() == nullptr));
    bblock->SetGraph(this);
    bblock->AddPredecessor(newPredecessor);
    newPredecessor->AddSuccessor(bblock);
    if (lastBlock == newPredecessor) {
        lastBlock = bblock;
    }
}

void Graph::UnlinkBasicBlock(BasicBlock *bblock) {
    ASSERT((bblock) && (bblock->GetGraph() == this));
    ASSERT(std::find(bblocks.begin(), bblocks.end(), bblock) != bblocks.end());
    bblocks.at(bblock->GetId()) = nullptr;
    bblock->SetId(BasicBlock::INVALID_ID);
}

void Graph::RemoveUnlinkedBlocks() {
    std::vector<BasicBlock *> newBBlocks;
    newBBlocks.reserve(bblocks.size());
    std::copy_if(bblocks.cbegin(), bblocks.cend(), newBBlocks.begin(), [](auto *b){ return b != nullptr; });
    for (size_t i = 0; i < newBBlocks.size(); ++i) {
        newBBlocks[i]->SetId(i);
    }
    bblocks = newBBlocks;
}

void Graph::removePredecessors(BasicBlock * /* bblock */) {
    UNREACHABLE("NOT IMPLEMENTED YET");
}

void Graph::removeSuccessors(BasicBlock * /* bblock */) {
    UNREACHABLE("NOT IMPLEMENTED YET");
}
}   // namespace ir
