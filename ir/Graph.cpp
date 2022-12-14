#include <algorithm>
#include "Graph.h"


namespace ir {
void Graph::ConnectBasicBlocks(BasicBlock *lhs, BasicBlock *rhs) {
    lhs->AddSuccessor(rhs);
    rhs->AddPredecessor(lhs);
}

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

// defined here after full declaration of Graph methods
BasicBlock::BasicBlock(Graph *graph)
    : id(INVALID_ID),
      preds(graph->GetAllocator()->ToSTL()),
      succs(graph->GetAllocator()->ToSTL()),
      firstPhi(nullptr),
      firstInst(nullptr),
      lastInst(nullptr),
      dominator(nullptr),
      dominated(graph->GetAllocator()->ToSTL()),
      loop(nullptr),
      graph(graph) {}
}   // namespace ir
