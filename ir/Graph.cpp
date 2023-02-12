#include <algorithm>
#include "Graph.h"


namespace ir {
size_t Graph::CountInstructions() const {
    size_t counter = 0;
    ForEachBasicBlock([&counter](const BasicBlock *bblock) { counter += bblock->GetSize(); });
    return counter;
}

BasicBlock *Graph::CreateEmptyBasicBlock(bool isTerminal) {
    auto *bblock = allocator->template New<BasicBlock>(this);
    AddBasicBlock(bblock);
    if (isTerminal) {
        if (!GetLastBasicBlock()) {
            SetLastBasicBlock(CreateEmptyBasicBlock(false));
        }
        ConnectBasicBlocks(bblock, GetLastBasicBlock());
    }
    return bblock;
}

void Graph::ConnectBasicBlocks(BasicBlock *lhs, BasicBlock *rhs) {
    ASSERT((lhs) && (rhs));
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
    ASSERT((bblock) && bblock->GetPredecessors().empty() && bblock->GetSuccessors().empty());

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
    unlinkBasicBlockImpl(bblock);
    if (bblock != GetLastBasicBlock()) {
        removePredecessors(bblock);
        removeSuccessors(bblock);
    } else {
        // TODO: replace `ifdef DEBUG` with `ifndef NDEBUG`
#ifdef DEBUG
        for (auto *pred : bblock->GetPredecessors()) {
            auto succs = pred->GetSuccessors();
            ASSERT(std::find(succs.begin(), succs.end(), bblock) == succs.end());
        }
        for (auto *succ : bblock->GetSuccessors()) {
            auto preds = succ->GetPredecessors();
            ASSERT(std::find(preds.begin(), preds.end(), bblock) == preds.end());
        }
#endif

        bblock->GetPredecessors().clear();
        bblock->GetSuccessors().clear();
    }
}

void Graph::unlinkBasicBlockImpl(BasicBlock *bblock) {
    ASSERT((bblock) && bblock->GetGraph() == this);
    auto id = bblock->GetId();
    ASSERT(id < bblocks.size() && bblocks[id] == bblock);
    bblocks[id] = nullptr;
    bblock->SetId(BasicBlock::INVALID_ID);
    ++unlinkedInstructionsCounter;
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

// defined here after full declaration of Graph's methods
BasicBlock::BasicBlock(Graph *graph)
    : id(INVALID_ID),
      preds(graph->GetAllocator()->ToSTL()),
      succs(graph->GetAllocator()->ToSTL()),
      dominated(graph->GetAllocator()->ToSTL()),
      graph(graph)
{}

bool BasicBlock::IsLastInGraph() const {
    return GetGraph()->GetLastBasicBlock() == this;
}
}   // namespace ir
