#include "LivenessAnalyzer.h"
#include "TestGraphSamples.h"


namespace ir::tests {
class LivenessAnalysisTest : public TestGraphSamples {
public:
    static constexpr OperandType TYPE = OperandType::U64;
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
         A
         |
         V
    ---->B
    |   / \
    |  /   \
    --C     D
            |
            V
            E
    */
    auto [graph, bblocks, expectedLinearOrder] = FillCase4();

    auto pass = LivenessAnalyzer(graph);
    pass.Run();

    checkLinearOrder(std::move(expectedLinearOrder), pass.GetLiveIntervals());
}

TEST_F(LivenessAnalysisTest, TestAnalysis2) {
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
    auto [graph, bblocks, linearOrder] = FillCase1();

    DumpGraphRPO(graph);
    auto pass = LivenessAnalyzer(graph);
    pass.Run();

    checkLinearOrder(std::move(linearOrder), pass.GetLiveIntervals());
}
}   // namespace ir::tests
