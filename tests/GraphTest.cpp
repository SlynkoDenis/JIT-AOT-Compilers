#include "gtest/gtest.h"
#include "InstructionBuilder.h"
#include "IRBuilder.h"
#include <iostream>

namespace ir::tests {
class GraphTest : public ::testing::Test {
public:
    virtual void SetUp() {
        irBuilder.CreateGraph();
    }

    GraphTest() = default;

    virtual void TearDown() {
        instrBuilder.Clear();
        irBuilder.Clear();
    }

    InstructionBuilder instrBuilder;
    IRBuilder irBuilder;
};

TEST_F(GraphTest, TestGraph1) {
    auto opType = OperandType::I32;
    auto vdest = VReg(0);
    auto vreg1 = VReg(1);
    auto vreg2 = VReg(2);
    auto *mul = instrBuilder.CreateMul(opType, vdest, vreg1, vreg2);

    // add a single instruction in the 1st basic block
    auto *bblock = irBuilder.CreateEmptyBasicBlock();
    instrBuilder.PushBackInstruction(bblock, mul);
    ASSERT_EQ(mul->GetBasicBlock(), bblock);
    ASSERT_EQ(bblock->GetFirstInstruction(), mul);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);

    auto *addi1 = instrBuilder.CreateAddi(opType, vreg1, vreg1, 32);
    auto *addi2 = instrBuilder.CreateAddi(opType, vreg2, vreg2, 32);

    // add another instruction in the 2nd basic block, which must be predecessor of the 1st
    auto *predBBlock1 = irBuilder.CreateEmptyBasicBlock();
    instrBuilder.PushBackInstruction(predBBlock1, addi1);
    bblock->GetGraph()->AddBasicBlockBefore(bblock, predBBlock1);
    ASSERT_EQ(predBBlock1->GetFirstInstruction(), addi1);
    ASSERT_EQ(predBBlock1->GetLastInstruction(), addi1);
    auto succs = predBBlock1->GetSuccessors();
    ASSERT_EQ(succs.size(), 1);
    ASSERT_EQ(succs[0], bblock);
    auto preds = bblock->GetPredecessors();
    ASSERT_EQ(preds.size(), 1);
    ASSERT_EQ(preds[0], predBBlock1);

    // add instruction in the 3rd basic block, which must be between 1st and 2nd
    auto *predBBlock2 = irBuilder.CreateEmptyBasicBlock();
    instrBuilder.PushBackInstruction(predBBlock2, addi2);
    bblock->GetGraph()->AddBasicBlockAfter(predBBlock1, predBBlock2);
    ASSERT_EQ(predBBlock2->GetFirstInstruction(), addi2);
    ASSERT_EQ(predBBlock2->GetLastInstruction(), addi2);
    succs = predBBlock1->GetSuccessors();
    ASSERT_EQ(succs.size(), 1);
    ASSERT_EQ(succs[0], predBBlock2);
    preds = predBBlock1->GetPredecessors();
    ASSERT_TRUE(preds.empty());
    preds = bblock->GetPredecessors();
    ASSERT_EQ(preds.size(), 1);
    ASSERT_EQ(preds[0], predBBlock2);
    succs = predBBlock2->GetSuccessors();
    ASSERT_EQ(succs.size(), 1);
    ASSERT_EQ(succs[0], bblock);
}

TEST_F(GraphTest, TestGraph2) {
    auto opType = OperandType::I32;
    auto vdest = VReg(0);
    auto vreg1 = VReg(1);
    auto vreg2 = VReg(2);
    auto *mul = instrBuilder.CreateMul(opType, vdest, vreg1, vreg2);
    auto *addi1 = instrBuilder.CreateAddi(opType, vreg1, vreg1, 32);
    auto *addi2 = instrBuilder.CreateAddi(opType, vreg2, vreg2, 32);

    // create basic blocks as: [addi1] -> [addi2] -> [mul]
    auto *mulBBlock = irBuilder.CreateEmptyBasicBlock();
    instrBuilder.PushBackInstruction(mulBBlock, mul);

    auto *addiBBlock1 = irBuilder.CreateEmptyBasicBlock();
    instrBuilder.PushBackInstruction(addiBBlock1, addi1);
    irBuilder.GetGraph()->AddBasicBlockBefore(mulBBlock, addiBBlock1);

    auto *addiBBlock2 = irBuilder.CreateEmptyBasicBlock();
    instrBuilder.PushBackInstruction(addiBBlock2, addi2);
    irBuilder.GetGraph()->AddBasicBlockAfter(addiBBlock1, addiBBlock2);

    // unlink middle basic block and check results
    irBuilder.GetGraph()->UnlinkBasicBlock(addiBBlock2);
    ASSERT_TRUE(addiBBlock2->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock2->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());

    // unlink last basic block and check results
    irBuilder.GetGraph()->UnlinkBasicBlock(mulBBlock);
    ASSERT_TRUE(mulBBlock->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());

    // unlink the remaining basic block and check results
    irBuilder.GetGraph()->UnlinkBasicBlock(addiBBlock1);
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());
}
}   // namespace ir::tests
