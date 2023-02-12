#include "CompilerTestBase.h"


namespace ir::tests {
class InstructionsTest : public CompilerTestBase {
};

TEST_F(InstructionsTest, TestMul) {
    auto opType = OperandType::I32;
    auto *arg1 = GetInstructionBuilder().CreateARG(opType);
    auto *arg2 = GetInstructionBuilder().CreateARG(opType);

    auto *instr = GetInstructionBuilder().CreateMUL(opType, arg1, arg2);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::MUL);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestAddi) {
    auto opType = OperandType::U64;
    auto *arg = GetInstructionBuilder().CreateARG(opType);
    auto imm = 11UL;

    auto *instr = GetInstructionBuilder().CreateADDI(opType, arg, imm);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInput(), arg);
    ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestConst) {
    auto opType = OperandType::I64;
    auto imm = 12L;

    auto *instr = GetInstructionBuilder().CreateCONST(opType, imm);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetValue(), imm);
}

TEST_F(InstructionsTest, TestCast) {
    auto fromType = OperandType::I32;
    auto toType = OperandType::U8;
    auto *arg = GetInstructionBuilder().CreateARG(fromType);
    
    auto *instr = GetInstructionBuilder().CreateCAST(fromType, toType, arg);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CAST);
    ASSERT_EQ(instr->GetType(), fromType);
    ASSERT_EQ(instr->GetTargetType(), toType);
    ASSERT_EQ(instr->GetInput(), arg);
}

TEST_F(InstructionsTest, TestCmp) {
    auto opType = OperandType::U64;
    auto ccode = CondCode::LT;
    auto *arg1 = GetInstructionBuilder().CreateARG(opType);
    auto *arg2 = GetInstructionBuilder().CreateARG(opType);

    auto *instr = GetInstructionBuilder().CreateCMP(opType, ccode, arg1, arg2);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::CMP);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetCondCode(), ccode);
    ASSERT_EQ(instr->GetInput(0), arg1);
    ASSERT_EQ(instr->GetInput(1), arg2);
}

TEST_F(InstructionsTest, TestJumpCMP) {
    auto &irBuilder = GetIRBuilder();
    auto *graph = irBuilder.GetGraph();
    auto &instrBuilder = GetInstructionBuilder();
    auto opType = OperandType::I16;

    auto *bblockSource = irBuilder.CreateEmptyBasicBlock();
    graph->SetFirstBasicBlock(bblockSource);
    auto *lhs = instrBuilder.CreateARG(opType);
    auto *rhs = instrBuilder.CreateARG(opType);
    auto *cmp = instrBuilder.CreateCMP(opType, CondCode::GE, lhs, rhs);
    auto *jcmp = instrBuilder.CreateJCMP();
    instrBuilder.PushBackInstruction(bblockSource, lhs, rhs, cmp, jcmp);

    auto *bblockTrue = irBuilder.CreateEmptyBasicBlock();
    graph->ConnectBasicBlocks(bblockSource, bblockTrue);
    auto *bblockFalse = irBuilder.CreateEmptyBasicBlock();
    graph->ConnectBasicBlocks(bblockSource, bblockFalse);

    ASSERT_NE(jcmp, nullptr);
    ASSERT_EQ(jcmp->GetOpcode(), Opcode::JCMP);
    ASSERT_EQ(jcmp->GetType(), OperandType::I64);
    ASSERT_EQ(jcmp->GetTrueDestination(), bblockTrue);
    ASSERT_EQ(jcmp->GetFalseDestination(), bblockFalse);
}

TEST_F(InstructionsTest, TestJmp) {
    auto &irBuilder = GetIRBuilder();
    auto *graph = irBuilder.GetGraph();
    auto &instrBuilder = GetInstructionBuilder();

    auto *bblockSource = irBuilder.CreateEmptyBasicBlock();
    graph->SetFirstBasicBlock(bblockSource);
    auto *jmp = instrBuilder.CreateJMP();
    instrBuilder.PushBackInstruction(bblockSource, jmp);

    auto *bblockDest = irBuilder.CreateEmptyBasicBlock();
    graph->ConnectBasicBlocks(bblockSource, bblockDest);

    ASSERT_NE(jmp, nullptr);
    ASSERT_EQ(jmp->GetOpcode(), Opcode::JMP);
    ASSERT_EQ(jmp->GetType(), OperandType::I64);
    ASSERT_EQ(jmp->GetDestination(), bblockDest);
}

TEST_F(InstructionsTest, TestRet) {
    auto opType = OperandType::U8;
    auto *arg = GetInstructionBuilder().CreateARG(opType);

    auto *instr = GetInstructionBuilder().CreateRET(opType, arg);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::RET);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetInput(), arg);
}

TEST_F(InstructionsTest, TestCall) {
    auto &instrBuilder = GetInstructionBuilder();

    auto opType = OperandType::U8;
    FunctionID callTarget = 1;

    auto *call = instrBuilder.CreateCALL(opType, callTarget);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->GetOpcode(), Opcode::CALL);
    ASSERT_EQ(call->GetType(), opType);
    ASSERT_EQ(call->GetCallTarget(), callTarget);
    ASSERT_EQ(call->GetInputsCount(), 0);

    auto *arg0 = instrBuilder.CreateARG(OperandType::U16);
    auto *arg1 = instrBuilder.CreateARG(OperandType::I64);
    callTarget = 7;
    call = instrBuilder.CreateCALL(opType, callTarget, {arg0, arg1});
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->GetOpcode(), Opcode::CALL);
    ASSERT_EQ(call->GetType(), opType);
    ASSERT_EQ(call->GetCallTarget(), callTarget);
    ASSERT_EQ(call->GetInputsCount(), 2);
    ASSERT_EQ(call->GetInput(0), arg0);
    ASSERT_EQ(call->GetInput(1), arg1);
}

TEST_F(InstructionsTest, TestLoad) {
    auto opType = OperandType::U64;
    uint64_t addr = 44;

    auto *instr = GetInstructionBuilder().CreateLOAD(opType, addr);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::LOAD);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetValue(), addr);
}

TEST_F(InstructionsTest, TestStore) {
    auto opType = OperandType::U32;
    auto *storedValue = GetInstructionBuilder().CreateARG(opType);
    uint64_t addr = 34;

    auto *instr = GetInstructionBuilder().CreateSTORE(storedValue, addr);

    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->GetOpcode(), Opcode::STORE);
    ASSERT_EQ(instr->GetType(), opType);
    ASSERT_EQ(instr->GetValue(), addr);
    ASSERT_EQ(instr->GetInputsCount(), 1);
    ASSERT_EQ(instr->GetInput(0), storedValue);
}

TEST_F(InstructionsTest, TestPhi) {
    auto opType = OperandType::U16;
    size_t numArgs = 3;
    auto args = std::vector<InstructionBase *>();
    args.reserve(numArgs);
    for (size_t i = 0; i < numArgs; ++i) {
        args.push_back(GetInstructionBuilder().CreateARG(opType));
    }

    std::vector<BasicBlock *> bblocks{3, nullptr};
    for (auto &bblock : bblocks) {
        bblock = GetIRBuilder().CreateEmptyBasicBlock();
    }

    auto *instr = GetInstructionBuilder().CreatePHI(opType, args, bblocks);

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
