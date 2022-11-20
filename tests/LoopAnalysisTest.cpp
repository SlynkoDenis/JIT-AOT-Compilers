#include "DomTree.h"
#include "gtest/gtest.h"
#include "IRBuilder.h"
#include "LoopAnalyzer.h"
#include "TestGraphSamples.h"


namespace ir::tests {
class LoopAnalysisTest : public ::testing::Test, public TestGraphSamples {
public:
    virtual void SetUp() {
        irBuilder.CreateGraph();
    }

    LoopAnalysisTest() = default;

    virtual void TearDown() {
        irBuilder.Clear();
    }

public:
    LoopAnalyzer loopAnalyzer;
};

TEST_F(LoopAnalysisTest, TestLoops1) {
    auto preBuiltGraph = BuildCase1();
    auto *graph = preBuiltGraph.first;

    loopAnalyzer.Analyze(graph);

    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_TRUE(rootLoop->GetInnerLoops().empty());

    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), graph->GetBasicBlocksCount());
    for (auto *bblock : preBuiltGraph.second) {
        ASSERT_EQ(bblock->GetLoop(), rootLoop);
    }
}

TEST_F(LoopAnalysisTest, TestLoops2) {
    auto preBuiltGraph = BuildCase2();
    auto *graph = preBuiltGraph.first;
    auto bblocks = preBuiltGraph.second;

    loopAnalyzer.Analyze(graph);

    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    auto innerLoops = rootLoop->GetInnerLoops();
    ASSERT_EQ(innerLoops.size(), 1);

    auto *mainLoop = innerLoops[0];
    ASSERT_EQ(mainLoop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(mainLoop->IsRoot());
    ASSERT_EQ(mainLoop->GetHeader(), bblocks[1]);
    ASSERT_EQ(mainLoop->GetInnerLoops().size(), 2);
    ASSERT_EQ(mainLoop->GetBackEdges().size(), 1);
    ASSERT_EQ(mainLoop->GetBackEdges()[0], bblocks[7]);
    // only non-inner-mainLoop basic blocks are counted
    ASSERT_EQ(mainLoop->GetBasicBlocks().size(), 4);
    ASSERT_FALSE(mainLoop->IsIrreducible());

    for (auto *loop : mainLoop->GetInnerLoops()) {
        ASSERT_EQ(loop->GetOuterLoop(), mainLoop);
        ASSERT_FALSE(loop->IsRoot());
        ASSERT_TRUE(loop->GetInnerLoops().empty());
        ASSERT_EQ(loop->GetBasicBlocks().size(), 2);
        ASSERT_EQ(loop->GetBackEdges().size(), 1);
        if (loop->GetHeader() == bblocks[2]) {
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[3]);
        } else {
            ASSERT_EQ(loop->GetHeader(), bblocks[4]);
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[5]);
        }
        ASSERT_FALSE(loop->IsIrreducible());
    }
}

TEST_F(LoopAnalysisTest, TestLoops3) {
    auto preBuiltGraph = BuildCase3();
    auto *graph = preBuiltGraph.first;
    auto bblocks = preBuiltGraph.second;

    loopAnalyzer.Analyze(graph);

    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 2);

    for (auto *loop : rootLoop->GetInnerLoops()) {
        ASSERT_FALSE(loop->IsRoot());
        ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
        ASSERT_TRUE(loop->GetInnerLoops().empty());

        ASSERT_EQ(loop->GetBackEdges().size(), 1);
        if (loop->GetHeader() == bblocks[1]) {
            ASSERT_FALSE(loop->IsIrreducible());
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[5]);
            ASSERT_EQ(loop->GetBasicBlocks().size(), 3);
        } else {
            ASSERT_EQ(loop->GetHeader(), bblocks[2]);
            ASSERT_TRUE(loop->IsIrreducible());
            ASSERT_EQ(loop->GetBackEdges()[0], bblocks[6]);
            ASSERT_EQ(loop->GetBasicBlocks().size(), 1);
        }
    }
}

TEST_F(LoopAnalysisTest, TestLoops4) {
    /*
       A
       |
       B <------
      / \      |
     /   \     |
    C     D    |
           \   |
            \  |
             E--
    */
    std::vector<BasicBlock *> bblocks(5);
    for (auto &it : bblocks) {
        it = irBuilder.CreateEmptyBasicBlock();
    }
    auto *graph = irBuilder.GetGraph();
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[1]);

    loopAnalyzer.Analyze(graph);

    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), 2);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 1);

    auto *loop = rootLoop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[1]);
    ASSERT_TRUE(loop->GetInnerLoops().empty());
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[4]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 3);
    ASSERT_FALSE(loop->IsIrreducible());
}

TEST_F(LoopAnalysisTest, TestLoops5) {
    /*
       A
       |
       B<--
      /   |
     /    |
    C->E->F
     \ |
      \|
       D
    */
    std::vector<BasicBlock *> bblocks(6);
    for (auto &it : bblocks) {
        it = irBuilder.CreateEmptyBasicBlock();
    }
    auto *graph = irBuilder.GetGraph();
    graph->SetFirstBasicBlock(bblocks[0]);
    for (size_t i = 0; i < 3; ++i) {
        graph->ConnectBasicBlocks(bblocks[i], bblocks[i + 1]);
    }
    graph->ConnectBasicBlocks(bblocks[2], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[4], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[1]);

    loopAnalyzer.Analyze(graph);

    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), 2);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 1);

    auto *loop = rootLoop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[1]);
    ASSERT_TRUE(loop->GetInnerLoops().empty());
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[5]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 4);
    ASSERT_FALSE(loop->IsIrreducible());
}

TEST_F(LoopAnalysisTest, TestLoops6) {
    /*
          A<--------
          |        |
          B<-----  |
         / \    |  |
        /   \   |  |
       C     D  |  |
      / \   /   |  |
     /   \ /    |  |
    E     F     |  |
          |     |  |
          G------  |
          |        |
          H---------
    */
    std::vector<BasicBlock *> bblocks(8);
    for (auto &it : bblocks) {
        it = irBuilder.CreateEmptyBasicBlock();
    }
    auto *graph = irBuilder.GetGraph();
    graph->SetFirstBasicBlock(bblocks[0]);
    graph->ConnectBasicBlocks(bblocks[0], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[2]);
    graph->ConnectBasicBlocks(bblocks[1], bblocks[3]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[4]);
    graph->ConnectBasicBlocks(bblocks[2], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[3], bblocks[5]);
    graph->ConnectBasicBlocks(bblocks[5], bblocks[6]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[7]);
    graph->ConnectBasicBlocks(bblocks[6], bblocks[1]);
    graph->ConnectBasicBlocks(bblocks[7], bblocks[0]);

    loopAnalyzer.Analyze(graph);

    auto *rootLoop = graph->GetLoopTree();
    ASSERT_TRUE(rootLoop->IsRoot());
    ASSERT_EQ(rootLoop->GetOuterLoop(), nullptr);
    ASSERT_EQ(rootLoop->GetBasicBlocks().size(), 1);
    ASSERT_EQ(rootLoop->GetInnerLoops().size(), 1);

    auto *loop = rootLoop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[0]);
    ASSERT_EQ(loop->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[7]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 2);
    ASSERT_FALSE(loop->IsIrreducible());

    loop = loop->GetInnerLoops()[0];
    ASSERT_EQ(loop->GetOuterLoop(), rootLoop->GetInnerLoops()[0]);
    ASSERT_FALSE(loop->IsRoot());
    ASSERT_EQ(loop->GetHeader(), bblocks[1]);
    ASSERT_TRUE(loop->GetInnerLoops().empty());
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges()[0], bblocks[6]);
    ASSERT_EQ(loop->GetBasicBlocks().size(), 5);
    ASSERT_FALSE(loop->IsIrreducible());
}
}   // namespace ir::tests
