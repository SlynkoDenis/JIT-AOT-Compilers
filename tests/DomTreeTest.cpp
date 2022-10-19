#include "DomTree.h"
#include "gtest/gtest.h"
#include "IRBuilder.h"


namespace ir::tests {
class DomTreeTest : public ::testing::Test {
public:
    virtual void SetUp() {
        irBuilder.CreateGraph();
    }

    DomTreeTest() = default;

    virtual void TearDown() {
        irBuilder.Clear();
    }

    IRBuilder irBuilder;
    DomTreeBuilder domTreeBuilder;
};

static void checkDominatedBlocks(BasicBlock *bblock,
                                 const std::vector<BasicBlock *> &expected) {
    ASSERT_NE(bblock, nullptr);
    auto dominated = bblock->GetDominatedBlocks();
    ASSERT_EQ(dominated.size(), expected.size());
    if (expected.empty()) {
        return;
    }
    std::sort(dominated.begin(), dominated.end(), [](BasicBlock *lhs, BasicBlock *rhs){
        return lhs->GetId() < rhs->GetId();
    });
    for (size_t i = 0; i < dominated.size(); ++i) {
        ASSERT_EQ(dominated[i], expected[i]);
    }
}

TEST_F(DomTreeTest, TestBuilding1) {
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
    std::vector<BasicBlock *> bblocks(7);
    for (auto &it : bblocks) {
        it = irBuilder.CreateEmptyBasicBlock();
    }
    auto *graph = irBuilder.GetGraph();
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[3]);

    domTreeBuilder.Build(graph);

    // check dominators
    std::vector<BasicBlock *> expectedDominators{
        nullptr, bblocks[0], bblocks[1], bblocks[1], bblocks[5], bblocks[1], bblocks[5]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i]->GetDominator(), expectedDominators[i]);
    }

    // check dominated basic blocks
    std::vector<std::vector<BasicBlock *>> expectedDominatedBlocks(bblocks.size());
    expectedDominatedBlocks[0] = {bblocks[1]};
    expectedDominatedBlocks[1] = {bblocks[2], bblocks[3], bblocks[5]};
    expectedDominatedBlocks[5] = {bblocks[4], bblocks[6]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        checkDominatedBlocks(bblocks[i], expectedDominatedBlocks[i]);
    }
}

TEST_F(DomTreeTest, TestBuilding2) {
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
    std::vector<BasicBlock *> bblocks(11);
    for (auto &it : bblocks) {
        it = irBuilder.CreateEmptyBasicBlock();
    }
    auto *graph = irBuilder.GetGraph();
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

    domTreeBuilder.Build(graph);

    // check dominators
    std::vector<BasicBlock *> expectedDominators{
        nullptr, bblocks[0], bblocks[1], bblocks[2], bblocks[3], bblocks[4],
        bblocks[5], bblocks[6], bblocks[6], bblocks[1], bblocks[8]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i]->GetDominator(), expectedDominators[i]);
    }

    // check dominated basic blocks
    std::vector<std::vector<BasicBlock *>> expectedDominatedBlocks(bblocks.size());
    for (size_t i = 2; i < 6; ++i) {
        expectedDominatedBlocks[i] = {bblocks[i + 1]};
    }
    expectedDominatedBlocks[0] = {bblocks[1]};
    expectedDominatedBlocks[1] = {bblocks[2], bblocks[9]};
    expectedDominatedBlocks[6] = {bblocks[7], bblocks[8]};
    expectedDominatedBlocks[8] = {bblocks[10]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        checkDominatedBlocks(bblocks[i], expectedDominatedBlocks[i]);
    }
}

TEST_F(DomTreeTest, TestBuilding3) {
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
    std::vector<BasicBlock *> bblocks(9);
    for (auto &it : bblocks) {
        it = irBuilder.CreateEmptyBasicBlock();
    }
    auto *graph = irBuilder.GetGraph();
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

    domTreeBuilder.Build(graph);

    // check dominators
    std::vector<BasicBlock *> expectedDominators{
        nullptr, bblocks[0], bblocks[1], bblocks[1], bblocks[1],
        bblocks[4], bblocks[1], bblocks[5], bblocks[1]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        ASSERT_EQ(bblocks[i]->GetDominator(), expectedDominators[i]);
    }

    // check dominated basic blocks
    std::vector<std::vector<BasicBlock *>> expectedDominatedBlocks(bblocks.size());
    expectedDominatedBlocks[0] = {bblocks[1]};
    expectedDominatedBlocks[1] = {bblocks[2], bblocks[3], bblocks[4], bblocks[6], bblocks[8]};
    expectedDominatedBlocks[4] = {bblocks[5]};
    expectedDominatedBlocks[5] = {bblocks[7]};
    for (size_t i = 0; i < bblocks.size(); ++i) {
        checkDominatedBlocks(bblocks[i], expectedDominatedBlocks[i]);
    }
}
}   // namespace ir::tests
