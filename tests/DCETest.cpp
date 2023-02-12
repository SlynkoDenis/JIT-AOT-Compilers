#include "DCE.h"
#include "Graph.h"
#include "TestGraphSamples.h"
#include <vector>


namespace ir::tests {
class DCETest : public TestGraphSamples {
public:
    void SetUp() override {
        pass = new DCEPass(GetIRBuilder().CreateGraph());
    }

public:
    DCEPass *pass = nullptr;
};

TEST_F(DCETest, TestDCE1) {
    // v0 = arg0 * 2
    // v1 = arg0 * 3
    // v2 = v0 - 1
    // return v2
    //
    // v1 must be cleared by DCE
    auto *bblock = GetIRBuilder().CreateEmptyBasicBlock();
    GetIRBuilder().GetGraph()->SetFirstBasicBlock(bblock);

    auto type = OperandType::I32;
    auto &instrBuilder = GetInstructionBuilder();
    auto *arg = instrBuilder.CreateARG(type);
    auto *v0 = instrBuilder.CreateMULI(type, arg, 2);
    auto *v1 = instrBuilder.CreateMULI(type, arg, 3);
    auto *v2 = instrBuilder.CreateSUBI(type, v0, 1);
    auto *ret = instrBuilder.CreateRET(type, v2);
    instrBuilder.PushBackInstruction(bblock, arg, v0, v1, v2, ret);
    ASSERT_EQ(bblock->GetSize(), 5);

    pass->Run();

    CompilerTestBase::compareInstructions({arg, v0, v2, ret}, bblock);
    ASSERT_EQ(v1->GetBasicBlock(), nullptr);
    ASSERT_EQ(v0->GetNextInstruction(), v2);
}

TEST_F(DCETest, TestDCE2) {
    // v0 = arg0 * 2
    // if (arg1 == 0) {
    //     v1 = v0 - 1
    //     v2 = v1 * 2
    // } else {
    //     v0 = v0 * 2
    // }
    // return v0
    //
    // v1 and v2 must be cleared by DCE
    auto *graph = GetIRBuilder().GetGraph();
    auto &instrBuilder = GetInstructionBuilder();
    auto type = OperandType::I32;

    auto *bblockSource = GetIRBuilder().CreateEmptyBasicBlock();
    graph->SetFirstBasicBlock(bblockSource);

    auto *arg0 = instrBuilder.CreateARG(type);
    auto *arg1 = instrBuilder.CreateARG(type);
    auto *constZero = instrBuilder.CreateCONST(type, 0);
    auto *v0 = instrBuilder.CreateMULI(type, arg0, 2);
    auto *cmp = instrBuilder.CreateCMP(type, CondCode::EQ, arg1, constZero);
    auto *jcmp = instrBuilder.CreateJCMP();
    instrBuilder.PushBackInstruction(bblockSource, arg0, arg1, constZero, v0, cmp, jcmp);
    ASSERT_EQ(bblockSource->GetSize(), 6);

    auto *bblockTrue = GetIRBuilder().CreateEmptyBasicBlock();
    auto *v1 = instrBuilder.CreateSUBI(type, v0, 1);
    auto *v2 = instrBuilder.CreateMULI(type, v1, 2);
    instrBuilder.PushBackInstruction(bblockTrue, v1, v2);
    ASSERT_EQ(bblockTrue->GetSize(), 2);

    auto *bblockFalse = GetIRBuilder().CreateEmptyBasicBlock();
    auto *v3 = instrBuilder.CreateMULI(type, v0, 1);
    instrBuilder.PushBackInstruction(bblockFalse, v3);
    ASSERT_EQ(bblockFalse->GetSize(), 1);

    auto *bblockDest = GetIRBuilder().CreateEmptyBasicBlock();
    auto *v4 = instrBuilder.CreatePHI(type, {v0, v3}, {bblockSource, bblockFalse});
    auto *ret = instrBuilder.CreateRET(type, v4);
    instrBuilder.PushBackInstruction(bblockDest, v4, ret);
    ASSERT_EQ(bblockDest->GetSize(), 2);

    graph->ConnectBasicBlocks(bblockSource, bblockTrue);
    graph->ConnectBasicBlocks(bblockSource, bblockFalse);
    graph->ConnectBasicBlocks(bblockTrue, bblockDest);
    graph->ConnectBasicBlocks(bblockFalse, bblockDest);
    ASSERT_EQ(graph->GetBasicBlocksCount(), 4);

    pass->Run();

    ASSERT_EQ(graph->GetBasicBlocksCount(), 4);
    CompilerTestBase::compareInstructions({arg0, arg1, constZero, v0, cmp, jcmp}, bblockSource);
    CompilerTestBase::compareInstructions({}, bblockTrue);
    CompilerTestBase::compareInstructions({v3}, bblockFalse);
    CompilerTestBase::compareInstructions({v4, ret}, bblockDest);
}

TEST_F(DCETest, TestNoDCE) {
    // v0 = arg0 * 2
    // v1 = arg0 / arg1
    // v2 = v0 - 1
    // return v2
    //
    // v1 must NOT be cleared by DCE, as long as division may produce an exception
    auto *bblock = GetIRBuilder().CreateEmptyBasicBlock();
    GetIRBuilder().GetGraph()->SetFirstBasicBlock(bblock);

    auto type = OperandType::I32;
    auto &instrBuilder = GetInstructionBuilder();
    auto *arg0 = instrBuilder.CreateARG(type);
    auto *arg1 = instrBuilder.CreateARG(type);
    auto *v0 = instrBuilder.CreateMULI(type, arg0, 2);
    auto *v1 = instrBuilder.CreateDIV(type, arg0, arg1);
    auto *v2 = instrBuilder.CreateSUBI(type, v0, 1);
    auto *ret = instrBuilder.CreateRET(type, v2);
    instrBuilder.PushBackInstruction(bblock, arg0, arg1, v0, v1, v2, ret);
    ASSERT_EQ(bblock->GetSize(), 6);

    pass->Run();

    CompilerTestBase::compareInstructions({arg0, arg1, v0, v1, v2, ret}, bblock);
}
}   // namespace ir::tests
