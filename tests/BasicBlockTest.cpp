#include "CompilerTestBase.h"


namespace ir::tests {
class BasicBlockTest : public CompilerTestBase {
};

TEST_F(BasicBlockTest, TestBasicBlock1) {
    auto *bblock = GetIRBuilder().CreateEmptyBasicBlock();
    ASSERT_NE(bblock, nullptr);
    ASSERT_EQ(bblock->GetFirstInstruction(), nullptr);
    ASSERT_EQ(bblock->GetLastInstruction(), nullptr);
    ASSERT_EQ(bblock->GetGraph(), GetIRBuilder().GetGraph());

    auto opType = OperandType::I32;
    auto *mul = GetInstructionBuilder().CreateMUL(opType, nullptr, nullptr);

    // add 1st instruction
    bblock->PushBackInstruction(mul);
    ASSERT_EQ(mul->GetBasicBlock(), bblock);
    ASSERT_EQ(bblock->GetFirstInstruction(), mul);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);

    auto *addi1 = GetInstructionBuilder().CreateADDI(opType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder().CreateADDI(opType, nullptr, 32);

    // add 2nd instruction into start of the basic block
    bblock->PushForwardInstruction(addi2);
    ASSERT_EQ(addi2->GetBasicBlock(), bblock);
    ASSERT_EQ(bblock->GetFirstInstruction(), addi2);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);

    // add 3rd instruction into start of the basic block
    bblock->PushForwardInstruction(addi1);
    ASSERT_EQ(addi1->GetBasicBlock(), bblock);
    ASSERT_EQ(bblock->GetFirstInstruction(), addi1);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);
    ASSERT_EQ(addi1->GetNextInstruction(), addi2);
}

TEST_F(BasicBlockTest, TestBasicBlock2) {
    auto *bblock = GetIRBuilder().CreateEmptyBasicBlock();

    auto opType = OperandType::I32;
    auto *mul = GetInstructionBuilder().CreateMUL(opType, nullptr, nullptr);
    auto *addi1 = GetInstructionBuilder().CreateADDI(opType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder().CreateADDI(opType, nullptr, 32);

    // add all 3 instructions
    GetInstructionBuilder().PushBackInstruction(bblock, addi1, addi2, mul);
    // check correct order
    ASSERT_EQ(bblock->GetFirstInstruction(), addi1);
    ASSERT_EQ(addi1->GetNextInstruction(), addi2);
    ASSERT_EQ(addi2->GetNextInstruction(), mul);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);
}

TEST_F(BasicBlockTest, TestBasicBlock3) {
    auto opType = OperandType::I32;
    auto *mul = GetInstructionBuilder().CreateMUL(opType, nullptr, nullptr);
    auto *addi1 = GetInstructionBuilder().CreateADDI(opType, nullptr, 32);
    auto *addi2 = GetInstructionBuilder().CreateADDI(opType, nullptr, 32);

    // add all 3 instructions
    auto *bblock = GetIRBuilder().CreateEmptyBasicBlock();
    GetInstructionBuilder().PushBackInstruction(bblock, addi1, addi2, mul);
    // unlink addi2 (2/3 instruction)
    bblock->UnlinkInstruction(addi2);
    ASSERT_EQ(addi2->GetBasicBlock(), nullptr);
    ASSERT_EQ(addi2->GetPrevInstruction(), nullptr);
    ASSERT_EQ(addi2->GetNextInstruction(), nullptr);
    ASSERT_EQ(bblock->GetFirstInstruction(), addi1);
    ASSERT_EQ(addi1->GetNextInstruction(), mul);
    ASSERT_EQ(bblock->GetLastInstruction(), mul);
    ASSERT_EQ(mul->GetPrevInstruction(), addi1);

    // unlink mul
    bblock->UnlinkInstruction(mul);
    ASSERT_EQ(mul->GetBasicBlock(), nullptr);
    ASSERT_EQ(mul->GetPrevInstruction(), nullptr);
    ASSERT_EQ(mul->GetNextInstruction(), nullptr);
    ASSERT_EQ(bblock->GetFirstInstruction(), addi1);
    ASSERT_EQ(bblock->GetLastInstruction(), addi1);
    ASSERT_EQ(addi1->GetPrevInstruction(), nullptr);
    ASSERT_EQ(addi1->GetNextInstruction(), nullptr);
}
}   // namespace ir::tests
