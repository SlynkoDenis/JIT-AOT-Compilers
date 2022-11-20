#include "CompilerTestBase.h"


namespace ir::tests {
class InstructionsTest : public CompilerTestBase {
};

TEST_F(InstructionsTest, TestMul) {
    auto opType = OperandType::I32;
    auto *arg1 = GetInstructionBuilder().CreateArg(opType);
    auto *arg2 = GetInstructionBuilder().CreateArg(opType);

    auto *instr = GetInstructionBuilder().CreateMul(opType, arg1, arg2);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MUL);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto opType = OperandType::U64;
    auto *arg = GetInstructionBuilder().CreateArg(opType);
    auto imm = 11UL;

    auto *instr = GetInstructionBuilder().CreateAddi(opType, arg, imm);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInput(), arg);
    ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestConst) {
    auto opType = OperandType::I64;
    auto imm = 12L;

    auto *instr = GetInstructionBuilder().CreateConst(opType, imm);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestCast) {
    auto fromType = OperandType::I32;
    auto toType = OperandType::U8;
    auto *arg = GetInstructionBuilder().CreateArg(fromType);
    
    auto *instr = GetInstructionBuilder().CreateCast(fromType, toType, arg);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CAST);
    ASSERT_EQ(instr->GetType(), fromType);
    ASSERT_EQ(instr->GetTargetType(), toType);
    ASSERT_EQ(instr->GetInput(), arg);
}

TEST_F(InstructionsTest, TestCmp) {
    auto opType = OperandType::U64;
    auto ccode = CondCode::LT;
    auto *arg1 = GetInstructionBuilder().CreateArg(opType);
    auto *arg2 = GetInstructionBuilder().CreateArg(opType);

    auto *instr = GetInstructionBuilder().CreateCmp(opType, ccode, arg1, arg2);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CMP);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetCondCode(), ccode);
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestJa) {
    auto dest = 42;
    auto *instr = GetInstructionBuilder().CreateJa(dest);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JA);
    ASSERT_EQ(instr->GetType(), OperandType::I64);
    ASSERT_EQ(instr->GetValue(), dest);
}

TEST_F(InstructionsTest, TestJmp) {
    int64_t dest = 42;
    auto *instr = GetInstructionBuilder().CreateJmp(dest);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JMP);
    ASSERT_EQ(instr->GetType(), OperandType::I64);
    ASSERT_EQ(instr->GetValue(), dest);
}

TEST_F(InstructionsTest, TestRet) {
    auto opType = OperandType::U8;
    auto *arg = GetInstructionBuilder().CreateArg(opType);

    auto *instr = GetInstructionBuilder().CreateRet(opType, arg);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::RET);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInput(), arg);
}

TEST_F(InstructionsTest, TestPhi) {
    auto opType = OperandType::U16;
    size_t numArgs = 3;
    auto args = std::vector<InstructionBase *>();
    args.reserve(numArgs);
    for (size_t i = 0; i < numArgs; ++i) {
        args.push_back(GetInstructionBuilder().CreateArg(opType));
    }

    std::vector<BasicBlock *> bblocks{3, nullptr};
    for (auto &bblock : bblocks) {
        bblock = GetIRBuilder().CreateEmptyBasicBlock();
    }

    auto *instr = GetInstructionBuilder().CreatePhi(opType, args, bblocks);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(instr->GetType(), opType);
    auto inputs = instr->GetInputs();
    ASSERT_EQ(inputs.size(), numArgs);
    for (size_t i = 0; i < numArgs; ++i) {
        ASSERT_EQ(inputs[i], args[i]);
    }
}
}   // namespace ir::tests
