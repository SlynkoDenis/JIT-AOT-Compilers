#include "Compiler.h"


int main() {
    ir::Compiler compiler;
    auto *graph = compiler.CreateNewGraph();
    graph->SetFirstBasicBlock(graph->CreateEmptyBasicBlock());
    auto *instrBuilder = graph->GetInstructionBuilder();

    auto u64 = ir::OperandType::U64;

    // factorial function
    // basic blocks
    auto *firstBlock = graph->CreateEmptyBasicBlock();
    auto *preLoop = graph->CreateEmptyBasicBlock();
    auto *loop = graph->CreateEmptyBasicBlock();
    auto *done = graph->CreateEmptyBasicBlock();

    // basic blocks internals
    auto *inputArgument = instrBuilder->CreateARG(ir::OperandType::U32);
    instrBuilder->PushBackInstruction(firstBlock,
        inputArgument,
        instrBuilder->CreateCONST(u64, 1),
        instrBuilder->CreateCONST(u64, 2),
        instrBuilder->CreateCAST(ir::OperandType::U32, u64, inputArgument));

    auto *tmp = firstBlock->GetLastInstruction();   // u32tou64 v2, a0
    auto *phiInst = instrBuilder->CreatePHI(u64, {tmp->GetPrevInstruction()}, {firstBlock});
    instrBuilder->PushBackInstruction(preLoop,
        phiInst,
        instrBuilder->CreateCMP(u64, ir::CondCode::EQ, phiInst, tmp),
        instrBuilder->CreateJCMP());

    tmp = firstBlock->GetFirstInstruction()->GetNextInstruction();  // movi v0, 1
    phiInst = instrBuilder->CreatePHI(u64, {tmp}, {firstBlock});
    auto *phiInst2 = instrBuilder->CreatePHI(u64, {tmp->GetNextInstruction()}, {firstBlock});
    instrBuilder->PushBackInstruction(loop,
        phiInst,
        phiInst2,
        instrBuilder->CreateMUL(u64, phiInst, phiInst2),
        instrBuilder->CreateADDI(u64, phiInst2, 1UL),
        instrBuilder->CreateJMP());

    tmp = loop->GetFirstInstruction();  // mul v0, v0, v1
    loop->GetFirstPhiInstruction()->AddInput(tmp);
    preLoop->GetFirstPhiInstruction()->AddInput(tmp->GetNextInstruction());

    auto *donePhi = instrBuilder->CreatePHI(u64,
        {tmp, firstBlock->GetFirstInstruction()->GetNextInstruction()},
        {loop, firstBlock});
    instrBuilder->PushBackInstruction(done, donePhi);
    done->PushBackInstruction(instrBuilder->CreateRET(u64, done->GetFirstPhiInstruction()));

    // connect basic blocks
    graph->ConnectBasicBlocks(firstBlock, preLoop);
    graph->ConnectBasicBlocks(preLoop, loop);
    graph->ConnectBasicBlocks(preLoop, done);
    loop->AddSuccessor(preLoop);
    return 0;
}
