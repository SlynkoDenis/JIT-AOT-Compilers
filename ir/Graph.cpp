#include <algorithm>
#include "Graph.h"


namespace ir {
bool Graph::VerifyFirstBlock() const {
#ifndef NDEBUG
    if (!firstBlock) {
        return true;
    }
    ASSERT(firstBlock->GetFirstPhiInstruction() == nullptr);
    ASSERT(firstBlock->GetLastPhiInstruction() == nullptr);
    bool foundConst = false;
    for (auto *instr : *firstBlock) {
        if (instr->IsConst()) {
            foundConst = true;
        } else if (instr->GetOpcode() == Opcode::ARG) {
            ASSERT(!foundConst);
        } else {
            UNREACHABLE("First basic block must contain only sequence of ARG and CONST instructions");
        }
    }
#endif  // NDEBUG
    return true;
}

size_t Graph::CountInstructions() const {
    size_t counter = 0;
    ForEachBasicBlock([&counter](const BasicBlock *bblock) { counter += bblock->GetSize(); });
    return counter;
}

BasicBlock *Graph::CreateEmptyBasicBlock(bool isTerminal) {
    auto *bblock = New<BasicBlock>(this);
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
#ifndef NDEBUG
        for (auto *pred : bblock->GetPredecessors()) {
            auto succs = pred->GetSuccessors();
            ASSERT(std::find(succs.begin(), succs.end(), bblock) == succs.end());
        }
        for (auto *succ : bblock->GetSuccessors()) {
            auto preds = succ->GetPredecessors();
            ASSERT(std::find(preds.begin(), preds.end(), bblock) == preds.end());
        }
#endif  // NDEBUG

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
    bblock->SetGraph(nullptr);
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
      preds(graph->GetMemoryResource()),
      succs(graph->GetMemoryResource()),
      dominated(graph->GetMemoryResource()),
      graph(graph)
{}

bool BasicBlock::IsFirstInGraph() const {
    return GetGraph()->GetFirstBasicBlock() == this;
}

bool BasicBlock::IsLastInGraph() const {
    return GetGraph()->GetLastBasicBlock() == this;
}
}   // namespace ir
