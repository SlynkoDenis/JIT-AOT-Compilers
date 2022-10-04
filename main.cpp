#include "InstructionBuilder.h"
#include "IRBuilder.h"


int main() {
    ir::InstructionBuilder instrBuilder;
    ir::IRBuilder irBuilder;
    irBuilder.CreateGraph();
    auto *graph = irBuilder.GetGraph();
    graph->SetFirstBasicBlock(irBuilder.CreateEmptyBasicBlock());

    auto arg0 = ir::VReg(0);
    std::vector<ir::VReg> vregs;
    auto regsCount = 8;
    vregs.reserve(regsCount);
    for (int i = 0; i < regsCount; ++i) {
        vregs.push_back(ir::VReg(i + 1));
    }
    auto u64 = ir::OperandType::U64;

    // factorial function
    auto *preLoop = irBuilder.CreateEmptyBasicBlock();
    graph->AddBasicBlockAfter(graph->GetFirstBasicBlock(), preLoop);
    instrBuilder.PushBackInstruction(preLoop,
        instrBuilder.CreateMovi(u64, vregs[0], 1UL),
        instrBuilder.CreateMovi(u64, vregs[1], 2UL),
        instrBuilder.CreateCast(ir::OperandType::U32, u64, vregs[2], arg0));

    auto *done = irBuilder.CreateEmptyBasicBlock();
    auto *loop = irBuilder.CreateEmptyBasicBlock();
    graph->AddBasicBlockAfter(preLoop, loop);
    instrBuilder.PushBackInstruction(preLoop,
        instrBuilder.CreatePhi(u64, vregs[3], vregs[1], vregs[6]),
        instrBuilder.CreateCmp(u64, ir::CondCode::EQ, vregs[3], vregs[2]),
        instrBuilder.CreateJa(done->GetId()),
        instrBuilder.CreatePhi(u64, vregs[4], vregs[5], vregs[0]),
        instrBuilder.CreateMul(u64, vregs[5], vregs[4], vregs[3]),
        instrBuilder.CreateAddi(u64, vregs[6], vregs[3], 1UL),
        instrBuilder.CreateJmp(preLoop->GetId()));

    instrBuilder.PushBackInstruction(done,
        instrBuilder.CreatePhi(u64, vregs[7], vregs[0], vregs[5]),
        instrBuilder.CreateRet(u64, vregs[7]));
    return 0;
}
