#include "CompilerTestBase.h"


namespace ir::tests {
class GraphTest : public CompilerTestBase {
};

TEST_F(GraphTest, TestGraph1) {
    auto opType = OperandType::I32;
    auto *mul = GetInstructionBuilder().CreateMul(opType, nullptr, nullptr);

    // add a single instruction in the 1st basic block
    auto *bblock = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(bblock, mul);
    ASSERT_EQ(mul->GetBasicBlock(), bblock);
    ASSERT_EQ(bblock->GetFirstInstruction(), mul);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);

    auto *addi1 = GetInstructionBuilder().CreateAddi(opType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder().CreateAddi(opType, nullptr, 32);

    // add another instruction in the 2nd basic block, which must be predecessor of the 1st
    auto *predBBlock1 = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(predBBlock1, addi1);
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
    auto *predBBlock2 = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(predBBlock2, addi2);
    bblock->GetGraph()->AddBasicBlockBefore(bblock, predBBlock2);
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
    auto *mul = GetInstructionBuilder().CreateMul(opType, nullptr, nullptr);
    auto *addi1 = GetInstructionBuilder().CreateAddi(opType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder().CreateAddi(opType, nullptr, 32);

    // create basic blocks as: [addi1] -> [addi2] -> [mul]
    auto *mulBBlock = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(mulBBlock, mul);

    auto *addiBBlock1 = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(addiBBlock1, addi1);
    GetIRBuilder().GetGraph()->AddBasicBlockBefore(mulBBlock, addiBBlock1);

    auto *addiBBlock2 = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(addiBBlock2, addi2);
    GetIRBuilder().GetGraph()->AddBasicBlockBefore(mulBBlock, addiBBlock2);

    // unlink middle basic block and check results
    GetIRBuilder().GetGraph()->UnlinkBasicBlock(addiBBlock2);
    ASSERT_TRUE(addiBBlock2->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock2->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());

    // unlink last basic block and check results
    GetIRBuilder().GetGraph()->UnlinkBasicBlock(mulBBlock);
    ASSERT_TRUE(mulBBlock->GetSuccessors().empty());
    ASSERT_TRUE(mulBBlock->GetPredecessors().empty());
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());

    // unlink the remaining basic block and check results
    GetIRBuilder().GetGraph()->UnlinkBasicBlock(addiBBlock1);
    ASSERT_TRUE(addiBBlock1->GetSuccessors().empty());
    ASSERT_TRUE(addiBBlock1->GetPredecessors().empty());
}
}   // namespace ir::tests
