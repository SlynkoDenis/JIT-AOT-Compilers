#include "InstructionBuilder.h"
#include "IRBuilder.h"


int main() {
    ir::InstructionBuilder instrBuilder;
    ir::IRBuilder irBuilder;
    irBuilder.CreateGraph();
    auto *graph = irBuilder.GetGraph();
    graph->SetFirstBasicBlock(irBuilder.CreateEmptyBasicBlock());

    auto u64 = ir::OperandType::U64;

    // factorial function
    // basic blocks
    auto *firstBlock = irBuilder.CreateEmptyBasicBlock();
    auto *preLoop = irBuilder.CreateEmptyBasicBlock();
    auto *loop = irBuilder.CreateEmptyBasicBlock();
    auto *done = irBuilder.CreateEmptyBasicBlock();

    // basic blocks internals
    auto *inputArgument = instrBuilder.CreateArg(ir::OperandType::U32);
    instrBuilder.PushBackInstruction(firstBlock,
        inputArgument,
        instrBuilder.CreateConst(u64, 1),
        instrBuilder.CreateConst(u64, 2),
        instrBuilder.CreateCast(ir::OperandType::U32, u64, inputArgument));

    auto *tmp = firstBlock->GetLastInstruction();   // u32tou64 v2, a0
    auto *phiInst = instrBuilder.CreatePhi(u64, tmp->GetPrevInstruction());
    instrBuilder.PushBackInstruction(preLoop,
        phiInst,
        instrBuilder.CreateCmp(u64, ir::CondCode::EQ, phiInst, tmp),
        instrBuilder.CreateJa(loop->GetId()));

    tmp = firstBlock->GetFirstInstruction()->GetNextInstruction();  // movi v0, 1
    phiInst = instrBuilder.CreatePhi(u64, tmp);
    auto *phiInst2 = instrBuilder.CreatePhi(u64, tmp->GetNextInstruction());
    instrBuilder.PushBackInstruction(loop,
        phiInst,
        phiInst2,
        instrBuilder.CreateMul(u64, phiInst, phiInst2),
        instrBuilder.CreateAddi(u64, phiInst2, 1UL),
        instrBuilder.CreateJmp(preLoop->GetId()));

    tmp = loop->GetFirstInstruction();  // mul v0, v0, v1
    loop->GetFirstPhiInstruction()->AddInput(tmp);
    preLoop->GetFirstPhiInstruction()->AddInput(tmp->GetNextInstruction());

    instrBuilder.PushBackInstruction(done,
        instrBuilder.CreatePhi(u64, tmp, firstBlock->GetFirstInstruction()->GetNextInstruction()));
    done->PushBackInstruction(instrBuilder.CreateRet(u64, done->GetFirstPhiInstruction()));

    // connect basic blocks
    graph->ConnectBasicBlocks(firstBlock, preLoop);
    graph->ConnectBasicBlocks(preLoop, loop);
    graph->ConnectBasicBlocks(preLoop, done);
    loop->AddSuccessor(preLoop);
    return 0;
}
