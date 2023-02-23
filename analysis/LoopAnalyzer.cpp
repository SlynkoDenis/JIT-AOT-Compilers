#include "DomTree.h"
#include "LoopAnalyzer.h"
#include "Traversals.h"


namespace ir {
void LoopAnalyzer::Run() {
    if (graph->IsEmpty()) {
        return;
    }

    resetStructs();
    PassManager::Run<DomTreeBuilder>(graph);

    collectBackEdges();
    graph->ReleaseMarker(greyMarker);
    graph->ReleaseMarker(blackMarker);

    populateLoops();
    buildLoopTree();
}

void LoopAnalyzer::resetStructs() {
    blockId = 0;

    auto bblocksCount = graph->GetBasicBlocksCount();
    if (dfsBlocks == nullptr) {
        auto *allocator = graph->GetAllocator();
        dfsBlocks = allocator->template NewVector<BasicBlock *>(bblocksCount, nullptr);
        loops = allocator->template NewVector<Loop *>();
    } else {
        dfsBlocks->clear();
        dfsBlocks->clear();
        loops->clear();

        dfsBlocks->resize(bblocksCount, nullptr);
    }
    greyMarker = graph->GetNewMarker();
    blackMarker = graph->GetNewMarker();
}

void LoopAnalyzer::collectBackEdges() {
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

void LoopAnalyzer::buildLoopTree() {
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

    bblock->SetMarker(greyMarker);
    for (auto *succ : bblock->GetSuccessors()) {
        if (succ->IsMarkerSet(greyMarker)) {
            addLoopInfo(succ, bblock, allocator);
        } else if (!succ->IsMarkerSet(blackMarker)) {
            dfsBackEdgesSearch(succ, allocator);
        }
    }
    bblock->ClearMarker(greyMarker);
    bblock->SetMarker(blackMarker);

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
    blackMarker = graph->GetNewMarker();
    loop->GetHeader()->SetMarker(blackMarker);
    for (auto *backEdgeSource : loop->GetBackEdges()) {
        dfsPopulateLoops(loop, backEdgeSource);
    }
    graph->ReleaseMarker(blackMarker);
}

void LoopAnalyzer::dfsPopulateLoops(Loop *loop, BasicBlock *bblock) {
    ASSERT((loop) && (bblock));
    if (bblock->IsMarkerSet(blackMarker)) {
        return;
    }
    bblock->SetMarker(blackMarker);

    auto *blockLoop = bblock->GetLoop();    
    if (blockLoop == nullptr) {
        loop->AddBasicBlock(bblock);
    } else if (blockLoop->GetId() != loop->GetId() &&
               blockLoop->GetOuterLoop() == nullptr) {
        blockLoop->SetOuterLoop(loop);
        loop->AddInnerLoop(blockLoop);
    }

    for (auto *pred : bblock->GetPredecessors()) {
        dfsPopulateLoops(loop, pred);
    }
}
}   // namespace ir
