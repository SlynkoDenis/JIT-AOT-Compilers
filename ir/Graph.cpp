#include <algorithm>
#include "Graph.h"


namespace ir {
void Graph::AddBasicBlock(BasicBlock *bblock) {
    ASSERT(bblock);
    bblock->SetId(bblocks.size());
    bblocks.push_back(bblock);
    bblock->SetGraph(this);
}

void Graph::AddBasicBlockBefore(BasicBlock *before, BasicBlock *bblock) {
    ASSERT((before) && (before->GetGraph() == this));
    ASSERT(bblock);
    ASSERT(bblock->GetPredecessors().empty() && bblock->GetSuccessors().empty());

    bblock->SetGraph(this);
    for (auto *b : before->GetPredecessors()) {
        b->RemoveSuccessor(before);
        b->AddSuccessor(bblock);
        bblock->AddPredecessor(b);
    }
    before->GetPredecessors().clear();
    before->AddPredecessor(bblock);

    bblock->AddSuccessor(before);
}

void Graph::AddBasicBlockAfter(BasicBlock *after, BasicBlock *bblock) {
    ASSERT((after) && (after->GetGraph() == this));
    ASSERT(bblock);
    ASSERT(bblock->GetPredecessors().empty() && bblock->GetSuccessors().empty());

    bblock->SetGraph(this);
    for (auto *b : after->GetSuccessors()) {
        b->RemovePredecessor(after);
        b->AddPredecessor(bblock);
        bblock->AddSuccessor(b);
    }
    after->GetSuccessors().clear();
    after->AddSuccessor(bblock);

    bblock->AddPredecessor(after);
    if (lastBlock == after) {
        lastBlock = bblock;
    }
}

void Graph::UnlinkBasicBlock(BasicBlock *bblock) {
    ASSERT((bblock) && (bblock->GetGraph() == this));
    auto id = bblock->GetId();
    ASSERT(id < bblocks.size() && bblocks[id] == bblock);
    bblocks[id] = nullptr;
    bblock->SetId(BasicBlock::INVALID_ID);

    if (bblock == lastBlock) {
        SetLastBasicBlock(nullptr);
    }

    removePredecessors(bblock);
    removeSuccessors(bblock);
}

void Graph::RemoveUnlinkedBlocks() {
    UNREACHABLE("TBD");
}

void Graph::removePredecessors(BasicBlock *bblock) {
    for (auto *b : bblock->GetPredecessors()) {
        b->RemoveSuccessor(bblock);
    }
    bblock->GetPredecessors().clear();
}

void Graph::removeSuccessors(BasicBlock *bblock) {
    for (auto *b : bblock->GetSuccessors()) {
        b->RemovePredecessor(bblock);
    }
    bblock->GetSuccessors().clear();
}
}   // namespace ir
