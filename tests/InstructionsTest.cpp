#include "gtest/gtest.h"
#include "InstructionBuilder.h"
#include "IRBuilder.h"


namespace ir::tests {
class InstructionsTest : public ::testing::Test {
public:
    InstructionsTest() = default;

    virtual void TearDown() {
        instrBuilder.Clear();
        irBuilder.Clear();
    }

    InstructionBuilder instrBuilder;
    IRBuilder irBuilder;
};

TEST_F(InstructionsTest, TestMul) {
    auto opType = OperandType::I32;
    auto vdest = VReg(0);
    auto vreg1 = VReg(1);
    auto vreg2 = VReg(2);
    auto *instr = instrBuilder.CreateMul(opType, vdest, vreg1, vreg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MUL);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetDestVReg(), vdest);
    ASSERT_EQ(instr->GetVReg1(), vreg1);
    ASSERT_EQ(instr->GetVReg2(), vreg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto opType = OperandType::U64;
    auto vdest = VReg(0);
    auto vreg = VReg(1);
    auto imm = 11UL;
    auto *instr = instrBuilder.CreateAddi(opType, vdest, vreg, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetDestVReg(), vdest);
    ASSERT_EQ(instr->GetVReg(), vreg);
    ASSERT_EQ(instr->GetImm(), imm);
}

TEST_F(InstructionsTest, TestMovi) {
    auto opType = OperandType::I64;
    auto vdest = VReg(0);
    auto imm = 12L;
    auto *instr = instrBuilder.CreateMovi(opType, vdest, imm);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MOVI);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetDestVReg(), vdest);
    ASSERT_EQ(instr->GetImm(), imm);
}

TEST_F(InstructionsTest, TestCast) {
    auto fromType = OperandType::I32;
    auto toType = OperandType::U8;
    auto vdest = VReg(0);
    auto vreg = VReg(1);
    auto *instr = instrBuilder.CreateCast(fromType, toType, vdest, vreg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CAST);
    ASSERT_EQ(instr->GetType(), fromType);
    ASSERT_EQ(instr->GetTargetType(), toType);
    ASSERT_EQ(instr->GetDestVReg(), vdest);
    ASSERT_EQ(instr->GetVReg(), vreg);
}

TEST_F(InstructionsTest, TestCmp) {
    auto opType = OperandType::U64;
    auto ccode = CondCode::LT;
    auto vreg1 = VReg(0);
    auto vreg2 = VReg(1);
    auto *instr = instrBuilder.CreateCmp(opType, ccode, vreg1, vreg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CMP);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetCondCode(), ccode);
    ASSERT_EQ(instr->GetVReg1(), vreg1);
    ASSERT_EQ(instr->GetVReg2(), vreg2);
}

TEST_F(InstructionsTest, TestJa) {
    auto dest = 42;
    auto *instr = instrBuilder.CreateJa(dest);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JA);
    ASSERT_EQ(instr->GetType(), OperandType::I64);
    ASSERT_EQ(instr->GetImm(), dest);
}

TEST_F(InstructionsTest, TestJmp) {
    int64_t dest = 42;
    auto *instr = instrBuilder.CreateJmp(dest);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::JMP);
    ASSERT_EQ(instr->GetType(), OperandType::I64);
    ASSERT_EQ(instr->GetImm(), dest);
}

TEST_F(InstructionsTest, TestRet) {
    auto opType = OperandType::U8;
    auto vreg = VReg(0);
    auto *instr = instrBuilder.CreateRet(opType, vreg);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::RET);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetVReg(), vreg);
}

TEST_F(InstructionsTest, TestPhi) {
    auto opType = OperandType::U16;
    auto vdest = VReg(0);
    auto vreg1 = VReg(1);
    auto vreg2 = VReg(2);
    auto *instr = instrBuilder.CreatePhi(opType, vdest, vreg1, vreg2);
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetDestVReg(), vdest);
    ASSERT_EQ(instr->GetVReg1(), vreg1);
    ASSERT_EQ(instr->GetVReg2(), vreg2);
}
}   // namespace ir::tests
