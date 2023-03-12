#include <array>
#include "BranchElimination.h"
#include "TestGraphSamples.h"


namespace ir::tests {
class BranchEliminationTest : public TestGraphSamples {
};

TEST_F(BranchEliminationTest, TestSingleIf0) {
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
    auto type = OperandType::I32;
    auto [graph, bblocks] = BuildCase0();
    auto *instrBuilder = GetInstructionBuilder();

    auto *arg = instrBuilder->CreateARG(type);
    auto *constZero = instrBuilder->CreateCONST(type, 0);
    instrBuilder->PushBackInstruction(bblocks[0], arg, constZero);

    auto *constCmp = instrBuilder->CreateCMP(type, CondCode::EQ, arg, arg);
    auto *constJcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(bblocks[1], constCmp, constJcmp);

    auto *phiInput1 = instrBuilder->CreateADDI(type, arg, 1);
    instrBuilder->PushBackInstruction(bblocks[2], phiInput1);

    auto *phiInput2 = instrBuilder->CreateADDI(type, arg, 2);
    instrBuilder->PushBackInstruction(bblocks[3], phiInput2);

    auto *phi = instrBuilder->CreatePHI(
        type,
        {phiInput1, phiInput2},
        {bblocks[2], bblocks[3]});
    auto *ret = instrBuilder->CreateRET(type, phi);
    instrBuilder->PushBackInstruction(bblocks[4], phi, ret);

    PassManager::Run<BranchElimination>(graph);

    VerifyControlAndDataFlowGraphs(graph);
    ASSERT_TRUE(graph->IsAnalysisValid(AnalysisFlag::RPO));
    ASSERT_EQ(bblocks[3]->GetGraph(), nullptr);
    ASSERT_EQ(constCmp->GetBasicBlock(), nullptr);
    ASSERT_EQ(constJcmp->GetBasicBlock(), nullptr);
    ASSERT_EQ(phi->GetBasicBlock(), nullptr);
    ASSERT_EQ(ret->GetInput(0), phiInput1);
}

TEST_F(BranchEliminationTest, TestSingleIf1) {
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
    auto type = OperandType::I32;
    auto [graph, bblocks] = BuildCase1();
    auto *instrBuilder = GetInstructionBuilder();

    auto *arg0 = instrBuilder->CreateARG(type);
    auto *arg1 = instrBuilder->CreateARG(type);
    auto *constZero = instrBuilder->CreateCONST(type, 0);
    auto *constOne = instrBuilder->CreateCONST(type, 1);
    instrBuilder->PushBackInstruction(
        bblocks[0],
        arg0, arg1, constZero, constOne);

    auto *constCmp = instrBuilder->CreateCMP(type, CondCode::GE, constZero, constOne);
    auto *constJcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(bblocks[1], constCmp, constJcmp);

    auto *phiInput1 = instrBuilder->CreateADD(type, arg0, constOne);
    instrBuilder->PushBackInstruction(bblocks[2], phiInput1);

    auto *cmp = instrBuilder->CreateCMP(type, CondCode::NE, arg0, constOne);
    auto *jcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(bblocks[5], cmp, jcmp);

    auto *phiInput2 = instrBuilder->CreateADDI(type, arg0, 2);
    instrBuilder->PushBackInstruction(bblocks[4], phiInput2);

    auto *phiInput3 = instrBuilder->CreateSUBI(type, arg1, 3);
    instrBuilder->PushBackInstruction(bblocks[6], phiInput3);

    auto *phi = instrBuilder->CreatePHI(
        type,
        {phiInput1->ToInput(), phiInput2->ToInput(), phiInput3->ToInput()},
        {bblocks[2], bblocks[5], bblocks[6]});
    auto *ret = instrBuilder->CreateRET(type, phi);
    instrBuilder->PushBackInstruction(bblocks[3], phi, ret);

    PassManager::Run<BranchElimination>(graph);

    VerifyControlAndDataFlowGraphs(graph);
    ASSERT_TRUE(graph->IsAnalysisValid(AnalysisFlag::RPO));
    ASSERT_EQ(bblocks[2]->GetGraph(), nullptr);
    ASSERT_EQ(constCmp->GetBasicBlock(), nullptr);
    ASSERT_EQ(constJcmp->GetBasicBlock(), nullptr);
    ASSERT_EQ(phi->GetInputsCount(), 2);
}
}   // namespace ir::tests
