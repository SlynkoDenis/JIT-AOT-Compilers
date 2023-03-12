#include "TestGraphSamples.h"


namespace ir::tests {
std::pair<Graph *, std::vector<BasicBlock *>> TestGraphSamples::BuildCase0() {
    /*
       A
       |
       B
      / \
     /   \
    C     D
     \   /
      \ /
       E
       |
       F
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(6);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->SetLastBasicBlock(bblocks[5]);

    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[5]);

    return {graph, bblocks};
}

std::pair<Graph *, std::vector<BasicBlock *>> TestGraphSamples::BuildCase1() {
    /*
       A
       |
       B
      / \
     /   \
    C     F
    |    / \
    |   E   \
    |  /    |
    | /     |
    D<------G
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(7);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[3]);

    return {graph, bblocks};
}

std::pair<Graph *, std::vector<BasicBlock *>> TestGraphSamples::BuildCase2() {
    /*
        A
        |
    --->B-->J
    |   |  /
    |   | /
    |   |/
    |   C<--
    |   |  |
    |   D---
    |   |
    |   E<--
    |   |  |
    |   F---
    |   |
    H<--G-->I-->K
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(11);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    for (size_t i = 0; i < 7; ++i) {
        graph->ConnectBasicBlocks(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBasicBlocks(bblocks[3], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[9]);
    graph->ConnectBasicBlocks(bblocks[9], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[8]);
    graph->ConnectBasicBlocks(bblocks[8], bblocks[10]);

    return {graph, bblocks};
}

std::pair<Graph *, std::vector<BasicBlock *>> TestGraphSamples::BuildCase3() {
    /*
          A
          |
          |
    ----->B
    |    /|
    |   / |
    |  E  C<--
    |  |\ |  |
    |  | \|  |
    ---F  D  |
       |  |  |
       |  |  |
       H->G---
        \ |
         \|
          I
    */
    auto *graph = GetGraph();
    std::vector<BasicBlock *> bblocks(9);
    for (auto &it : bblocks) {
        it = graph->CreateEmptyBasicBlock();
    }
    graph->SetFirstBasicBlock(bblocks[0]);
    for (size_t i = 0; i < 3; ++i) {
        graph->ConnectBasicBlocks(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBasicBlocks(bblocks[1], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[7]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[8]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[8]);

    return {graph, bblocks};
}
}   // namespace ir::tests
