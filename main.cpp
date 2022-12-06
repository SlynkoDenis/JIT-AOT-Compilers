#include "Compiler.h"


int main() {
    Compiler compiler;
    compiler.GetIRBuilder().CreateGraph();
    auto *graph = compiler.GetIRBuilder().GetGraph();
    graph->SetFirstBasicBlock(compiler.GetIRBuilder().CreateEmptyBasicBlock());

    auto u64 = ir::OperandType::U64;

    // factorial function
    // basic blocks
    auto *firstBlock = compiler.GetIRBuilder().CreateEmptyBasicBlock();
    auto *preLoop = compiler.GetIRBuilder().CreateEmptyBasicBlock();
    auto *loop = compiler.GetIRBuilder().CreateEmptyBasicBlock();
    auto *done = compiler.GetIRBuilder().CreateEmptyBasicBlock();

    // basic blocks internals
    auto *inputArgument = compiler.GetInstructionBuilder().CreateARG(ir::OperandType::U32);
    compiler.GetInstructionBuilder().PushBackInstruction(firstBlock,
        inputArgument,
        compiler.GetInstructionBuilder().CreateCONST(u64, 1),
        compiler.GetInstructionBuilder().CreateCONST(u64, 2),
        compiler.GetInstructionBuilder().CreateCAST(ir::OperandType::U32, u64, inputArgument));

    auto *tmp = firstBlock->GetLastInstruction();   // u32tou64 v2, a0
    auto *phiInst = compiler.GetInstructionBuilder().CreatePHI(u64, {tmp->GetPrevInstruction()}, {firstBlock});
    compiler.GetInstructionBuilder().PushBackInstruction(preLoop,
        phiInst,
        compiler.GetInstructionBuilder().CreateCMP(u64, ir::CondCode::EQ, phiInst, tmp),
        compiler.GetInstructionBuilder().CreateJA(loop->GetId()));

    tmp = firstBlock->GetFirstInstruction()->GetNextInstruction();  // movi v0, 1
    phiInst = compiler.GetInstructionBuilder().CreatePHI(u64, {tmp}, {firstBlock});
    auto *phiInst2 = compiler.GetInstructionBuilder().CreatePHI(u64, {tmp->GetNextInstruction()}, {firstBlock});
    compiler.GetInstructionBuilder().PushBackInstruction(loop,
        phiInst,
        phiInst2,
        compiler.GetInstructionBuilder().CreateMUL(u64, phiInst, phiInst2),
        compiler.GetInstructionBuilder().CreateADDI(u64, phiInst2, 1UL),
        compiler.GetInstructionBuilder().CreateJMP(preLoop->GetId()));

    tmp = loop->GetFirstInstruction();  // mul v0, v0, v1
    loop->GetFirstPhiInstruction()->AddInput(tmp);
    preLoop->GetFirstPhiInstruction()->AddInput(tmp->GetNextInstruction());

    auto *donePhi = compiler.GetInstructionBuilder().CreatePHI(u64,
        {tmp, firstBlock->GetFirstInstruction()->GetNextInstruction()},
        {loop, firstBlock});
    compiler.GetInstructionBuilder().PushBackInstruction(done, donePhi);
    done->PushBackInstruction(compiler.GetInstructionBuilder().CreateRET(u64, done->GetFirstPhiInstruction()));

    // connect basic blocks
    graph->ConnectBasicBlocks(firstBlock, preLoop);
    graph->ConnectBasicBlocks(preLoop, loop);
    graph->ConnectBasicBlocks(preLoop, done);
    loop->AddSuccessor(preLoop);
    return 0;
}
