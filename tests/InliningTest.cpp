#include "Graph.h"
#include "Inlining.h"
#include "TestGraphSamples.h"
#include "Traversals.h"
#include <vector>


namespace ir::tests {
class InliningTest : public CompilerTestBase {
public:
    CallInstruction *BuildCallerGraph(bool voidReturn);

    Graph *BuildSimpleCallee();
    Graph *BuildMultipleReturnsCallee();
    Graph *BuildVoidReturnCallee();

    void RunPass() {
        auto *graph = GetGraph();
        PassManager::Run<InliningPass>(graph);
        ASSERT_FALSE(graph->IsAnalysisValid(AnalysisFlag::DOM_TREE));
        ASSERT_FALSE(graph->IsAnalysisValid(AnalysisFlag::LOOP_ANALYSIS));
    }

public:
    static constexpr bool SHOULD_DUMP = true;
    static constexpr auto OPS_TYPE = OperandType::I32;
};

CallInstruction *InliningTest::BuildCallerGraph(bool voidReturn) {
    // int32 testFunction(int32 curFibValue, int32 prevFibValue, int32 argFibNumber) {
    //     if (argFibNumber >= 0) {
    //         int32 newFibValue = curFibValue + prevFibValue
    //         int32 decrementedFibNumber = argFibNumber - 1
    //         int32 recursiveCall = testFunction(newFibValue, curFibValue, decrementedFibNumber)
    //         return recursiveCall OR return (void)
    //     } else {
    //         int32 finalCall = functionToInline(curFibValue, prevFibValue)
    //         return finallCall OR return (void)
    //     }
    // }
    auto *instrBuilder = GetInstructionBuilder();

    auto *firstBlock = GetGraph()->CreateEmptyBasicBlock();
    GetGraph()->SetFirstBasicBlock(firstBlock);
    auto *curFibValue = instrBuilder->CreateARG(OPS_TYPE);
    auto *prevFibValue = instrBuilder->CreateARG(OPS_TYPE);
    auto *argFibNumber = instrBuilder->CreateARG(OPS_TYPE);
    auto *constZero = instrBuilder->CreateCONST(OPS_TYPE, 0);
    auto *cmp = instrBuilder->CreateCMP(OPS_TYPE, CondCode::GE, argFibNumber, constZero);
    auto *jcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(
        firstBlock,
        curFibValue, prevFibValue, argFibNumber, constZero, cmp, jcmp);

    auto *trueBranch = GetGraph()->CreateEmptyBasicBlock(true);
    GetGraph()->ConnectBasicBlocks(firstBlock, trueBranch);
    auto *newFibValue = instrBuilder->CreateADD(OPS_TYPE, curFibValue, prevFibValue);
    auto *decrementedFibNumber = instrBuilder->CreateSUBI(OPS_TYPE, argFibNumber, 1);
    std::pmr::vector<Input> args(GetGraph()->GetMemoryResource());
    args.push_back(newFibValue);
    args.push_back(curFibValue);
    args.push_back(decrementedFibNumber);
    auto *recursiveCall = instrBuilder->CreateCALL(
        voidReturn ? OperandType::VOID : OPS_TYPE, GetGraph()->GetId(), args);
    InstructionBase *recursiveRet = nullptr;
    if (voidReturn) {
        recursiveRet = instrBuilder->CreateRETVOID();
    } else {
        recursiveRet = instrBuilder->CreateRET(OPS_TYPE, recursiveCall);
    }
    instrBuilder->PushBackInstruction(
        trueBranch,
        newFibValue, decrementedFibNumber, recursiveCall, recursiveRet);

    auto *falseBranch = GetGraph()->CreateEmptyBasicBlock(true);
    GetGraph()->ConnectBasicBlocks(firstBlock, falseBranch);
    auto callType = voidReturn ? OperandType::VOID : OPS_TYPE;
    auto *finalCall = instrBuilder->CreateCALL(
        callType,
        INVALID_FUNCTION_ID,
        {curFibValue, prevFibValue});
    InstructionBase *finalRet = nullptr;
    if (voidReturn) {
        finalRet = instrBuilder->CreateRETVOID();
        callType = OperandType::VOID;
    } else {
        finalRet = instrBuilder->CreateRET(OPS_TYPE, finalCall);
    }
    instrBuilder->PushBackInstruction(
        falseBranch,
        finalCall, finalRet);

    return finalCall;
}

Graph *InliningTest::BuildSimpleCallee() {
    // int32 functionToInline(int32 arg0, int32 arg1) {
    //     int32 sub = arg1 - arg0
    //     int32 mul = sub * 3
    //     return mul
    // }
    auto *calleeGraph = compiler.CreateNewGraph();
    auto *instrBuilder = GetInstructionBuilder(calleeGraph);

    auto *bblock = calleeGraph->CreateEmptyBasicBlock(true);
    calleeGraph->SetFirstBasicBlock(bblock);
    auto *arg0 = instrBuilder->CreateARG(OPS_TYPE);
    auto *arg1 = instrBuilder->CreateARG(OPS_TYPE);
    auto *sub = instrBuilder->CreateSUB(OPS_TYPE, arg1, arg0);
    auto *mul = instrBuilder->CreateMULI(OPS_TYPE, sub, 3);
    auto *ret = instrBuilder->CreateRET(OPS_TYPE, mul);
    instrBuilder->PushBackInstruction(bblock, arg0, arg1, sub, mul, ret);

    return calleeGraph;
}

Graph *InliningTest::BuildMultipleReturnsCallee() {
    // int32 functionToInline(int32 arg0, int32 arg1) {
    //     int32 modiByTwo = arg1 % 2
    //     if (modiByTwo == 0) {
    //         int32 diviByTwo = arg1 / 2
    //         return diviByTwo
    //     } else {
    //         int32 modByThree = arg0 % 3
    //         if (modByThree == 0) {
    //             int32 divByThree = arg0 / 3
    //             return divByThree
    //         } else {
    //             return arg0
    //         }
    //     }
    // }
    auto *calleeGraph = compiler.CreateNewGraph();
    auto *instrBuilder = GetInstructionBuilder(calleeGraph);

    auto *firstBlock = calleeGraph->CreateEmptyBasicBlock();
    calleeGraph->SetFirstBasicBlock(firstBlock);
    auto *arg0 = instrBuilder->CreateARG(OPS_TYPE);
    auto *arg1 = instrBuilder->CreateARG(OPS_TYPE);
    auto *constZero = instrBuilder->CreateCONST(OPS_TYPE, 0);
    auto *constThree = instrBuilder->CreateCONST(OPS_TYPE, 3);
    auto *modiByTwo = instrBuilder->CreateMODI(OPS_TYPE, arg1, 2);
    auto *cmp = instrBuilder->CreateCMP(OPS_TYPE, CondCode::EQ, modiByTwo, constZero);
    auto *jcmp = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(firstBlock, arg0, arg1, constZero, constThree, modiByTwo, cmp, jcmp);

    auto *trueBranch = calleeGraph->CreateEmptyBasicBlock(true);
    auto *diviByTwo = instrBuilder->CreateDIVI(OPS_TYPE, arg1, 2);
    auto *ret1 = instrBuilder->CreateRET(OPS_TYPE, diviByTwo);
    instrBuilder->PushBackInstruction(trueBranch, diviByTwo, ret1);
    calleeGraph->ConnectBasicBlocks(firstBlock, trueBranch);

    auto *falseBranch = calleeGraph->CreateEmptyBasicBlock();
    auto *modByThree = instrBuilder->CreateMOD(OPS_TYPE, arg0, constThree);
    auto *cmp2 = instrBuilder->CreateCMP(OPS_TYPE, CondCode::EQ, modByThree, constZero);
    auto *jcmp2 = instrBuilder->CreateJCMP();
    instrBuilder->PushBackInstruction(falseBranch, modByThree, cmp2, jcmp2);
    calleeGraph->ConnectBasicBlocks(firstBlock, falseBranch);

    auto *trueBranch2 = calleeGraph->CreateEmptyBasicBlock(true);
    auto *divByThree = instrBuilder->CreateDIV(OPS_TYPE, arg0, constThree);
    auto *ret2 = instrBuilder->CreateRET(OPS_TYPE, divByThree);
    instrBuilder->PushBackInstruction(trueBranch2, divByThree, ret2);
    calleeGraph->ConnectBasicBlocks(falseBranch, trueBranch2);

    auto *falseBranch2 = calleeGraph->CreateEmptyBasicBlock(true);
    auto *ret3 = instrBuilder->CreateRET(OPS_TYPE, arg0);
    instrBuilder->PushBackInstruction(falseBranch2, ret3);
    calleeGraph->ConnectBasicBlocks(falseBranch, falseBranch2);

    return calleeGraph;
}

Graph *InliningTest::BuildVoidReturnCallee() {
    // int32 functionToInline(int32 arg0, int32 arg1) {
    //     int32 sub = arg1 - arg0
    //     int32 div = sub / arg1
    //     return (void)
    // }
    auto *calleeGraph = compiler.CreateNewGraph();
    auto *instrBuilder = GetInstructionBuilder(calleeGraph);

    auto *bblock = calleeGraph->CreateEmptyBasicBlock(true);
    calleeGraph->SetFirstBasicBlock(bblock);
    auto *arg0 = instrBuilder->CreateARG(OPS_TYPE);
    auto *arg1 = instrBuilder->CreateARG(OPS_TYPE);
    auto *sub = instrBuilder->CreateSUB(OPS_TYPE, arg1, arg0);
    // div for side-effects
    auto *div = instrBuilder->CreateDIV(OPS_TYPE, sub, arg1);
    auto *retVoid = instrBuilder->CreateRETVOID();
    instrBuilder->PushBackInstruction(bblock, arg0, arg1, sub, div, retVoid);

    return calleeGraph;
}

TEST_F(InliningTest, TestInlineSimple) {
    ASSERT_EQ(GetGraph()->GetBasicBlocksCount(), 0);
    auto *call = BuildCallerGraph(false);
    auto *callerGraph = GetGraph();
    size_t callerBlocksCount = 4;
    ASSERT_EQ(callerGraph->GetBasicBlocksCount(), callerBlocksCount);
    ASSERT_EQ(callerGraph->CountInstructions(), 12);

    auto *calleeGraph = BuildSimpleCallee();
    size_t calleeBlocksCount = 2;
    ASSERT_EQ(calleeGraph->GetBasicBlocksCount(), calleeBlocksCount);
    ASSERT_EQ(calleeGraph->CountInstructions(), 5);
    call->SetCallTarget(calleeGraph->GetId());
    ASSERT_EQ(compiler.GetFunction(call->GetCallTarget()), calleeGraph);

    RunPass();

    ASSERT_EQ(callerGraph->GetBasicBlocksCount(), 2 * callerBlocksCount + calleeBlocksCount - 1);
}

TEST_F(InliningTest, TestInlineMultipleReturns) {
    auto *call = BuildCallerGraph(false);
    auto *callerGraph = GetGraph();
    size_t callerBlocksCount = 4;
    ASSERT_EQ(callerGraph->GetBasicBlocksCount(), callerBlocksCount);
    ASSERT_EQ(callerGraph->CountInstructions(), 12);

    auto *calleeGraph = BuildMultipleReturnsCallee();
    size_t calleeBlocksCount = 6;
    ASSERT_EQ(calleeGraph->GetBasicBlocksCount(), calleeBlocksCount);
    ASSERT_EQ(calleeGraph->CountInstructions(), 15);
    call->SetCallTarget(calleeGraph->GetId());
    ASSERT_EQ(compiler.GetFunction(call->GetCallTarget()), calleeGraph);

    RunPass();

    ASSERT_EQ(callerGraph->GetBasicBlocksCount(),
              2 * callerBlocksCount + calleeBlocksCount + 2);
}

TEST_F(InliningTest, TestInlineVoidReturn) {
    auto *call = BuildCallerGraph(true);
    auto *callerGraph = GetGraph();
    size_t callerBlocksCount = 4;
    ASSERT_EQ(callerGraph->GetBasicBlocksCount(), callerBlocksCount);
    ASSERT_EQ(callerGraph->CountInstructions(), 12);

    auto *calleeGraph = BuildVoidReturnCallee();
    size_t calleeBlocksCount = 2;
    ASSERT_EQ(calleeGraph->GetBasicBlocksCount(), calleeBlocksCount);
    ASSERT_EQ(calleeGraph->CountInstructions(), 5);
    call->SetCallTarget(calleeGraph->GetId());
    ASSERT_EQ(compiler.GetFunction(call->GetCallTarget()), calleeGraph);

    RunPass();

    ASSERT_EQ(callerGraph->GetBasicBlocksCount(), 2 * callerBlocksCount + calleeBlocksCount - 1);
}
}   // namespace ir::tests
