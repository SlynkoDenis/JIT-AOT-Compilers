#include "DomTree.h"
#include "LoopAnalyzer.h"
#include "Traversals.h"


namespace ir {
void LoopAnalyzer::Analyze(Graph *graph) {
    ASSERT(graph);
    if (graph->IsEmpty()) {
        return;
    }

    resetStructs(graph);
    DomTreeBuilder().Build(graph);
    collectBackEdges(graph);
    populateLoops();
    buildLoopTree(graph);
}

void LoopAnalyzer::resetStructs(Graph *graph) {
    colorCounter = static_cast<uint32_t>(DFSColors::COLORS_SIZE);
    blockId = 0;

    auto bblocksCount = graph->GetBasicBlocksCount();
    if (dfsColors == nullptr) {
        auto *allocator = graph->GetAllocator();
        dfsColors = allocator->template NewVector<DFSColors>(bblocksCount, DFSColors::WHITE);
        dfsBlocks = allocator->template NewVector<BasicBlock *>(bblocksCount, nullptr);
        loops = allocator->template NewVector<Loop *>();
    } else {
        dfsBlocks->clear();
        dfsBlocks->clear();
        loops->clear();

        dfsColors->resize(bblocksCount, DFSColors::WHITE);
        dfsBlocks->resize(bblocksCount, nullptr);
    }
}

void LoopAnalyzer::collectBackEdges(Graph *graph) {
    ASSERT(graph);
    dfsBackEdgesSearch(graph->GetFirstBasicBlock(), graph->GetAllocator());
    ASSERT(blockId == graph->GetBasicBlocksCount());
}

void LoopAnalyzer::populateLoops() {
    for (auto *bblock : *dfsBlocks) {
        auto *loop = bblock->GetLoop();
        if (loop == nullptr || loop->GetHeader() != bblock) {
            continue;
        }

        if (loop->IsIrreducible()) {
            for (auto *backEdgeSource : loop->GetBackEdges()) {
                if (backEdgeSource->GetLoop() == nullptr) {
                    backEdgeSource->SetLoop(loop);
                }
            }
        } else {
            populateReducibleLoop(loop);
        }
    }
}

void LoopAnalyzer::buildLoopTree(Graph *graph) {
    ASSERT(graph);
    auto *allocator = graph->GetAllocator();
    auto *rootLoop = allocator->template New<Loop>(loops->size(), nullptr, false, allocator, true);
    loops->push_back(rootLoop);

    for (auto *bblock : *dfsBlocks) {
        auto *loop = bblock->GetLoop();
        if (loop == nullptr) {
            rootLoop->AddBasicBlock(bblock);
        } else if (loop->GetOuterLoop() == nullptr) {
            loop->SetOuterLoop(rootLoop);
            rootLoop->AddInnerLoop(loop);
        }
    }

    graph->SetLoopTree(rootLoop);
}

void LoopAnalyzer::dfsBackEdgesSearch(BasicBlock *bblock, ArenaAllocator *const allocator) {
    ASSERT(bblock);

    dfsColors->at(bblock->GetId()) = DFSColors::GREY;
    for (auto *succ : bblock->GetSuccessors()) {
        auto &color = dfsColors->at(succ->GetId());
        if (color == DFSColors::WHITE) {
            dfsBackEdgesSearch(succ, allocator);
        } else if (color == DFSColors::GREY) {
            addLoopInfo(succ, bblock, allocator);
        }
    }
    dfsColors->at(bblock->GetId()) = DFSColors::BLACK;

    dfsBlocks->at(blockId++) = bblock;
}

void LoopAnalyzer::addLoopInfo(BasicBlock *header, BasicBlock *backEdgeSource,
                               ArenaAllocator *const allocator) {
    auto *loop = header->GetLoop();
    if (loop == nullptr) {
        loop = allocator->template New<Loop>(loops->size(), header,
                                             isLoopIrreducible(header, backEdgeSource), allocator);
        loop->AddBasicBlock(header);
        loop->AddBackEdge(backEdgeSource);
        loops->push_back(loop);

        header->SetLoop(loop);
    } else {
        loop->AddBackEdge(backEdgeSource);
        if (isLoopIrreducible(header, backEdgeSource)) {
            loop->SetIrreducibility(true);
        }
    }
}

void LoopAnalyzer::populateReducibleLoop(Loop *loop) {
    ASSERT(loop);
    // reuse `dfsColors` vector in this DFS
    auto color = static_cast<DFSColors>(++colorCounter);
    dfsColors->at(loop->GetHeader()->GetId()) = color;
    for (auto *backEdgeSource : loop->GetBackEdges()) {
        dfsPopulateLoops(loop, backEdgeSource, color);
    }
}

void LoopAnalyzer::dfsPopulateLoops(Loop *loop, BasicBlock *bblock, DFSColors color) {
    ASSERT((loop) && (bblock));
    if (dfsColors->at(bblock->GetId()) == color) {
        return;
    }
    dfsColors->at(bblock->GetId()) = color;

    auto *blockLoop = bblock->GetLoop();    
    if (blockLoop == nullptr) {
        loop->AddBasicBlock(bblock);
    } else if (blockLoop->GetId() != loop->GetId() &&
               blockLoop->GetOuterLoop() == nullptr) {
        blockLoop->SetOuterLoop(loop);
        loop->AddInnerLoop(blockLoop);
    }

    for (auto *pred : bblock->GetPredecessors()) {
        dfsPopulateLoops(loop, pred, color);
    }
}
}   // namespace ir
