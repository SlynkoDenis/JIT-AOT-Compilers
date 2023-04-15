#include "LivenessAnalyzer.h"
#include "TestGraphSamples.h"


namespace ir::tests {
class LivenessAnalysisTest : public TestGraphSamples {
};

template <typename AllocLhsT, typename AllocRhsT>
static void checkLinearOrder(std::vector<LiveIntervals, AllocLhsT> &&expectedLinearOrder,
                             const std::vector<LiveIntervals, AllocRhsT> &liveIntervals)
{
    ASSERT_EQ(liveIntervals.size(), expectedLinearOrder.size());
    for (int i = 0, end = liveIntervals.size(); i < end; ++i) {
        ASSERT_EQ(liveIntervals[i].GetInstruction(), expectedLinearOrder[i].GetInstruction());
        ASSERT_EQ(liveIntervals[i], expectedLinearOrder[i]);
    }
}

TEST_F(LivenessAnalysisTest, TestAnalysis1) {
    /*
         B0
         |
         V
    ---->B1
    |   / \
    |  /   \
    --B2    B3
            |
            V
            B4
    */
    auto [graph, bblocks, expectedLinearOrder] = FillCase4();

    auto pass = LivenessAnalyzer(graph);
    pass.Run();

    checkLinearOrder(std::move(expectedLinearOrder), pass.GetLiveIntervals());
}

TEST_F(LivenessAnalysisTest, TestAnalysis2) {
    /*
       B0
       |
       B1
      / \
     /   \
    B2   B5
    |    / \
    |   B4  \
    |  /     |
    | /      |
    B3<------B6
    */
    auto [graph, bblocks, linearOrder] = FillCase1();

    auto pass = LivenessAnalyzer(graph);
    pass.Run();

    checkLinearOrder(std::move(linearOrder), pass.GetLiveIntervals());
}

TEST_F(LivenessAnalysisTest, TestAnalysis3) {
    /*
          B0
          |
          B1<-------
          |        |
          B2<----  |
         / \    |  |
        /   \   |  |
       B3   B4  |  |
      / \   /   |  |
     /   \ /    |  |
    B5    B6    |  |
          |     |  |
          B7-----  |
          |        |
          B8--------
          |
          B9
    */
    auto [graph, bblocks, linearOrder] = FillCase5();

    auto pass = LivenessAnalyzer(graph);
    pass.Run();

    checkLinearOrder(std::move(linearOrder), pass.GetLiveIntervals());
}
}   // namespace ir::tests
