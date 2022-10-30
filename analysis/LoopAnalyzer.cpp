#include "DomTree.h"
#include "LoopAnalyzer.h"
#include "Traversals.h"


namespace ir {
void LoopAnalyzer::Analyze(Graph *graph) {
    ASSERT(graph);
    if (graph->IsEmpty()) {
        return;
    }

    resetStructs(graph->GetBasicBlocksCount());
    DomTreeBuilder().Build(graph);
    collectBackEdges(graph);
    populateLoops();
    buildLoopTree(graph);
}

void LoopAnalyzer::resetStructs(size_t bblocksCount) {
    colorCounter = static_cast<uint32_t>(DFSColors::COLORS_SIZE);
    dfsColors.resize(bblocksCount, DFSColors::WHITE);

    blockId = 0;
    dfsBlocks.resize(bblocksCount, nullptr);

    loops.clear();
}

void LoopAnalyzer::collectBackEdges(Graph *graph) {
    ASSERT(graph);
    dfsBackEdgesSearch(graph->GetFirstBasicBlock());
    ASSERT(blockId == graph->GetBasicBlocksCount());
}

void LoopAnalyzer::populateLoops() {
    for (auto *bblock : dfsBlocks) {
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

    auto *rootLoop = new Loop(loops.size(), nullptr, false, true);
    loops.push_back(rootLoop);

    for (auto *bblock : dfsBlocks) {
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

void LoopAnalyzer::dfsBackEdgesSearch(BasicBlock *bblock) {
    ASSERT(bblock);

    dfsColors[bblock->GetId()] = DFSColors::GREY;
    for (auto *succ : bblock->GetSuccessors()) {
        auto &color = dfsColors[succ->GetId()];
        if (color == DFSColors::WHITE) {
            dfsBackEdgesSearch(succ);
        } else if (color == DFSColors::GREY) {
            addLoopInfo(succ, bblock);
        }
    }
    dfsColors[bblock->GetId()] = DFSColors::BLACK;

    dfsBlocks[blockId++] = bblock;
}

void LoopAnalyzer::addLoopInfo(BasicBlock *header, BasicBlock *backEdgeSource) {
    auto *loop = header->GetLoop();
    if (loop == nullptr) {
        loop = new Loop(loops.size(), header, isLoopIrreducible(header, backEdgeSource));
        loop->AddBasicBlock(header);
        loop->AddBackEdge(backEdgeSource);
        loops.push_back(loop);

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
    dfsColors[loop->GetHeader()->GetId()] = color;
    for (auto *backEdgeSource : loop->GetBackEdges()) {
        dfsPopulateLoops(loop, backEdgeSource, color);
    }
}

void LoopAnalyzer::dfsPopulateLoops(Loop *loop, BasicBlock *bblock, DFSColors color) {
    ASSERT((loop) && (bblock));
    if (dfsColors[bblock->GetId()] == color) {
        return;
    }
    dfsColors[bblock->GetId()] = color;

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
