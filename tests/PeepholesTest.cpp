#include "CompilerTestBase.h"
#include "Peephole.h"


namespace ir::tests {
class PeepholesTest : public CompilerTestBase {
public:
    void SetUp() override {
        CompilerTestBase::SetUp();
        pass = new PeepholePass(GetGraph(), SHOULD_DUMP);
    }
    void TearDown() override {
        delete pass;
        CompilerTestBase::TearDown();
    }

public:
    static constexpr bool SHOULD_DUMP = true;

public:
    PeepholePass *pass = nullptr;
};

// AND

TEST_F(PeepholesTest, TestAND1) {
    // case:
    // v0 = 0
    // v2 = v1 & v0
    // expected:
    // v2 is replaced with v0
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constZero = GetInstructionBuilder()->CreateCONST(opType, 0);
    auto *andInstr = GetInstructionBuilder()->CreateAND(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, andInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, constZero, arg, andInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
    compareInstructions({constZero, arg, userInstr}, bblock);
    ASSERT_EQ(bblock->GetLastInstruction(), userInstr);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

TEST_F(PeepholesTest, TestAND2) {
    // case:
    // v0 = 0xffff (max value of this type)
    // v2 = v1 & v0
    // expected:
    // v2 is replaced with v1
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constMax = GetInstructionBuilder()->CreateCONST(opType, std::numeric_limits<int32_t>::max());
    auto *andInstr = GetInstructionBuilder()->CreateAND(opType, arg, constMax);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, andInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, constMax, arg, andInstr, userInstr);

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    compareInstructions({constMax, arg, userInstr}, bblock);
}

TEST_F(PeepholesTest, TestANDWithNEGArgs) {
    // case:
    // v2 = ~v0 & ~v1
    // expected:
    // v3 = v0 | v1
    // v2 = ~v3
    auto opType = OperandType::I32;
    auto *arg1 = GetInstructionBuilder()->CreateARG(opType);
    auto *arg2 = GetInstructionBuilder()->CreateARG(opType);
    auto *not1 = GetInstructionBuilder()->CreateNOT(opType, arg1);
    auto *not2 = GetInstructionBuilder()->CreateNOT(opType, arg2);
    auto *andInstr = GetInstructionBuilder()->CreateAND(opType, not1, not2);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, andInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg1, arg2, not1, not2, andInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
    ASSERT_EQ(bblock->GetLastInstruction(), userInstr);
    auto *notInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_NE(notInstr, nullptr);
    ASSERT_EQ(notInstr->GetOpcode(), Opcode::NOT);
    auto *orInstr = static_cast<UnaryRegInstruction *>(notInstr)->GetInput(0).GetInstruction();
    ASSERT_EQ(orInstr->GetOpcode(), Opcode::OR);
    auto *typed = static_cast<BinaryRegInstruction *>(orInstr);
    if (typed->GetInput(0) == arg1) {
        ASSERT_EQ(typed->GetInput(1), arg2);
    } else {
        ASSERT_EQ(typed->GetInput(0), arg2);
        ASSERT_EQ(typed->GetInput(1), arg1);
    }
}

TEST_F(PeepholesTest, TestANDRepeatedArgs) {
    // case:
    // v1 = v0 & v0
    // expected:
    // v1 is replaced with v0
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *andInstr = GetInstructionBuilder()->CreateAND(opType, arg, arg);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, andInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg, andInstr, userInstr);

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    compareInstructions({arg, userInstr}, bblock);
}

TEST_F(PeepholesTest, TestANDFolding) {
    // case:
    // v0 = imm1
    // v1 = imm2
    // v2 = v0 & v1
    // expected:
    // v2 is replaced with CONST instruction with the corresponding value
    auto opType = OperandType::I32;
    int imm1 = 12;
    int imm2 = 34;
    auto *const1 = GetInstructionBuilder()->CreateCONST(opType, imm1);
    auto *const2 = GetInstructionBuilder()->CreateCONST(opType, imm2);
    auto *andInstr = GetInstructionBuilder()->CreateAND(opType, const1, const2);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, const1, const2, andInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize);
    auto *newInstr = bblock->GetLastInstruction();
    ASSERT_NE(newInstr, nullptr);
    ASSERT_TRUE(newInstr->IsConst());
    ASSERT_EQ(static_cast<ConstantInstruction *>(newInstr)->GetValue(), imm1 & imm2);
}

#define TEST_CONST_ARG_NO_OPTIMIZATIONS(name, imm_val)                                  \
TEST_F(PeepholesTest, TestNoOptimization##name) {                                       \
    auto opType = OperandType::I32;                                                     \
    auto *arg = GetInstructionBuilder()->CreateARG(opType);                              \
    auto *constInstr = GetInstructionBuilder()->CreateCONST(opType, imm_val);            \
    auto *targetInstr = GetInstructionBuilder()->Create##name(opType, arg, constInstr);  \
    auto *bblock = GetGraph()->CreateEmptyBasicBlock();                              \
    GetGraph()->SetFirstBasicBlock(bblock);                              \
    GetInstructionBuilder()->PushBackInstruction(bblock, constInstr, arg, targetInstr);  \
    pass->Run();                                                                        \
    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);                                             \
    compareInstructions({constInstr, arg, targetInstr}, bblock);                        \
}

TEST_CONST_ARG_NO_OPTIMIZATIONS(AND, 44);

// SRA

TEST_F(PeepholesTest, TestZeroSRA) {
    // case:
    // v0 = 0
    // v2 = v0 >> v1
    // expected:
    // v2 is replaced with v0
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constZero = GetInstructionBuilder()->CreateCONST(opType, 0);
    auto *sraInstr = GetInstructionBuilder()->CreateSRA(opType, constZero, arg);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, sraInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg, constZero, sraInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
    compareInstructions({arg, constZero, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), constZero);
}

TEST_F(PeepholesTest, TestSRAZero) {
    // case:
    // v0 = 0
    // v2 = v1 >> v0
    // expected:
    // v2 is replaced with v1
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constZero = GetInstructionBuilder()->CreateCONST(opType, 0);
    auto *sraInstr = GetInstructionBuilder()->CreateSRA(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, sraInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg, constZero, sraInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
    compareInstructions({arg, constZero, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), arg);
}

static void testSRAFolding(InstructionBuilder *instrBuilder, Graph *graph,
                           PeepholePass *pass, int imm1, int imm2) {
    // case:
    // v1 = imm1
    // v2 = imm2
    // v3 = v1 >> v2
    // expected:
    // v3 is replaced with CONST instruction with the corresponding value
    auto opType = OperandType::I32;
    auto *const1 = instrBuilder->CreateCONST(opType, imm1);
    auto *const2 = instrBuilder->CreateCONST(opType, imm2);
    auto *sraInstr = instrBuilder->CreateSRA(opType, const1, const2);

    auto *bblock = graph->CreateEmptyBasicBlock();
    graph->SetFirstBasicBlock(bblock);
    instrBuilder->PushBackInstruction(bblock, const1, const2, sraInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize);
    auto *newInstr = bblock->GetLastInstruction();
    ASSERT_NE(newInstr, nullptr);
    ASSERT_TRUE(newInstr->IsConst());
    ASSERT_EQ(static_cast<ConstantInstruction *>(newInstr)->GetValue(), imm1 >> imm2);
}

TEST_F(PeepholesTest, TestSRAFolding1) {
    testSRAFolding(GetInstructionBuilder(), GetGraph(), pass, 12345, 3);
}
TEST_F(PeepholesTest, TestSRAFolding2) {
    testSRAFolding(GetInstructionBuilder(), GetGraph(), pass, 12345, 0);
}
TEST_F(PeepholesTest, TestSRAFolding3) {
    testSRAFolding(GetInstructionBuilder(), GetGraph(), pass, -12345, 3);
}
TEST_F(PeepholesTest, TestSRAFolding4) {
    testSRAFolding(GetInstructionBuilder(), GetGraph(), pass, -12345, 0);
}

TEST_F(PeepholesTest, TestSRANoOptimization) {
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constInstr = GetInstructionBuilder()->CreateCONST(opType, 43);
    auto *andInstr = GetInstructionBuilder()->CreateSRA(opType, arg, constInstr);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, constInstr, arg, andInstr);

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    compareInstructions({constInstr, arg, andInstr}, bblock);
}

TEST_CONST_ARG_NO_OPTIMIZATIONS(SRA, 3);

// SUB

TEST_F(PeepholesTest, TestSUB1) {
    // case:
    // v2 = -v1
    // v3 = v0 - v2
    // expected:
    // v2 = -v1
    // v3 = v0 + v1
    auto opType = OperandType::I32;
    auto *arg1 = GetInstructionBuilder()->CreateARG(opType);
    auto *arg2 = GetInstructionBuilder()->CreateARG(opType);
    auto *negInstr = GetInstructionBuilder()->CreateNEG(opType, arg2);
    auto *subInstr = GetInstructionBuilder()->CreateSUB(opType, arg1, negInstr);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg1, arg2, negInstr, subInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize);
    auto *newInstr = bblock->GetLastInstruction();
    ASSERT_NE(newInstr, nullptr);
    ASSERT_TRUE(newInstr->GetOpcode() == Opcode::ADD);
    auto *typed = static_cast<BinaryRegInstruction *>(newInstr);
    if (typed->GetInput(0) == arg1) {
        ASSERT_EQ(typed->GetInput(1), arg2);
    } else {
        ASSERT_EQ(typed->GetInput(0), arg2);
        ASSERT_EQ(typed->GetInput(1), arg1);
    }
}

static void testAddSubCombination(InstructionBuilder *instrBuilder, Graph *graph,
                                  PeepholePass *pass, bool firstAdd, bool firstFromAdd) {
    auto opType = OperandType::I32;
    auto *arg1 = instrBuilder->CreateARG(opType);
    auto *arg2 = instrBuilder->CreateARG(opType);
    auto *addInstr = instrBuilder->CreateADD(opType, arg1, arg2);

    InstructionBase *fromAdd = firstFromAdd ? arg1 : arg2;
    InstructionBase *firstArg = addInstr;
    InstructionBase *secondArg = fromAdd;
    if (!firstAdd) {
        firstArg = fromAdd;
        secondArg = addInstr;
    }
    auto *subInstr = instrBuilder->CreateSUB(opType, firstArg, secondArg);
    auto *userInstr = instrBuilder->CreateADDI(opType, subInstr, 123);

    auto *bblock = graph->CreateEmptyBasicBlock();
    graph->SetFirstBasicBlock(bblock);
    instrBuilder->PushBackInstruction(bblock, arg1, arg2, addInstr, subInstr, userInstr);

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(userInstr, bblock->GetLastInstruction());
    auto *remainedInstr = firstFromAdd ? arg2 : arg1;
    if (firstAdd) {
        CompilerTestBase::compareInstructions({arg1, arg2, addInstr, userInstr}, bblock);
        ASSERT_EQ(userInstr->GetInput(0), remainedInstr);
    } else {
        auto *newInstr = userInstr->GetPrevInstruction();
        CompilerTestBase::compareInstructions({arg1, arg2, addInstr, newInstr, userInstr}, bblock);
        ASSERT_NE(newInstr, nullptr);
        ASSERT_EQ(newInstr->GetOpcode(), Opcode::NEG);
        auto newInstrArg = static_cast<UnaryRegInstruction *>(newInstr)->GetInput(0);
        ASSERT_EQ(newInstrArg, remainedInstr);
    }
}

TEST_F(PeepholesTest, TestSUB2) {
    testAddSubCombination(GetInstructionBuilder(), GetGraph(), pass, true, true);
}
TEST_F(PeepholesTest, TestSUB3) {
    testAddSubCombination(GetInstructionBuilder(), GetGraph(), pass, true, false);
}
TEST_F(PeepholesTest, TestSUB4) {
    testAddSubCombination(GetInstructionBuilder(), GetGraph(), pass, false, true);
}
TEST_F(PeepholesTest, TestSUB5) {
    testAddSubCombination(GetInstructionBuilder(), GetGraph(), pass, false, false);
}

TEST_F(PeepholesTest, TestZeroSUB) {
    // case:
    // v1 = 0
    // v2 = v1 - v0
    // expected:
    // v1 = 0
    // v2 = -v0
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constZero = GetInstructionBuilder()->CreateCONST(opType, 0);
    auto *subInstr = GetInstructionBuilder()->CreateSUB(opType, constZero, arg);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, subInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg, constZero, subInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize);
    ASSERT_EQ(bblock->GetLastInstruction(), userInstr);
    auto *newInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_NE(newInstr, nullptr);
    ASSERT_EQ(newInstr->GetOpcode(), Opcode::NEG);
    ASSERT_EQ(static_cast<UnaryRegInstruction *>(newInstr)->GetInput(0), arg);
}

TEST_F(PeepholesTest, TestSUBZero) {
    // case:
    // v1 = 0
    // v2 = v0 - v1
    // expected:
    // v2 is replaced with v0
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *constZero = GetInstructionBuilder()->CreateCONST(opType, 0);
    auto *subInstr = GetInstructionBuilder()->CreateSUB(opType, arg, constZero);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, subInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg, constZero, subInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize - 1);
    compareInstructions({arg, constZero, userInstr}, bblock);
    ASSERT_EQ(userInstr->GetInput(0), arg);
}

TEST_F(PeepholesTest, TestSUBRepeatedArgs) {
    // case:
    // v1 = v0 - v0
    // expected:
    // v1 is replaced with CONST instruction with the zero value
    auto opType = OperandType::I32;
    auto *arg = GetInstructionBuilder()->CreateARG(opType);
    auto *subInstr = GetInstructionBuilder()->CreateSUB(opType, arg, arg);
    auto *userInstr = GetInstructionBuilder()->CreateADDI(opType, subInstr, 123);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg, subInstr, userInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize);
    ASSERT_EQ(bblock->GetLastInstruction(), userInstr);
    auto *newInstr = userInstr->GetInput(0).GetInstruction();
    ASSERT_NE(newInstr, nullptr);
    ASSERT_TRUE(newInstr->IsConst());
    auto *typed = static_cast<ConstantInstruction *>(newInstr);
    ASSERT_EQ(typed->GetValue(), 0);
}

TEST_F(PeepholesTest, TestSUBFolding) {
    // case:
    // v1 = imm1
    // v2 = imm2
    // v3 = v1 - v2
    // expected:
    // v3 is replaced with CONST instruction with the corresponding value
    auto opType = OperandType::I32;
    int imm1 = 12;
    int imm2 = 15;
    auto *const1 = GetInstructionBuilder()->CreateCONST(opType, imm1);
    auto *const2 = GetInstructionBuilder()->CreateCONST(opType, imm2);
    auto *subInstr = GetInstructionBuilder()->CreateSUB(opType, const1, const2);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, const1, const2, subInstr);
    auto prevSize = bblock->GetSize();

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    ASSERT_EQ(bblock->GetSize(), prevSize);
    auto *newInstr = bblock->GetLastInstruction();
    ASSERT_NE(newInstr, nullptr);
    ASSERT_TRUE(newInstr->IsConst());
    auto *typed = static_cast<ConstantInstruction *>(newInstr);
    ASSERT_EQ(typed->GetValue(), imm1 - imm2);
}

TEST_F(PeepholesTest, TestAddSubNoOptimizations) {
    auto opType = OperandType::I32;
    auto *arg1 = GetInstructionBuilder()->CreateARG(opType);
    auto *arg2 = GetInstructionBuilder()->CreateARG(opType);
    auto *arg3 = GetInstructionBuilder()->CreateARG(opType);
    auto *addInstr = GetInstructionBuilder()->CreateADD(opType, arg1, arg2);
    auto *subInstr = GetInstructionBuilder()->CreateSUB(opType, addInstr, arg3);

    auto *bblock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(bblock);
    GetInstructionBuilder()->PushBackInstruction(bblock, arg1, arg2, arg3, addInstr, subInstr);

    pass->Run();

    CompilerTestBase::VerifyControlAndDataFlowGraphs(bblock);
    compareInstructions({arg1, arg2, arg3, addInstr, subInstr}, bblock);
}

TEST_CONST_ARG_NO_OPTIMIZATIONS(SUB, 33);

#undef TEST_CONST_ARG_NO_OPTIMIZATIONS
}   // namespace ir::tests
