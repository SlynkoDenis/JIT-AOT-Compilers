#include "InstructionBuilder.h"
#include "IRBuilder.h"


int main() {
    ir::InstructionBuilder instrBuilder;
    ir::IRBuilder irBuilder;
    irBuilder.CreateGraph();
    auto *bblock = irBuilder.CreateEmptyBasicBlock();
    auto vreg = ir::VReg(0);
    auto vdest = ir::VReg(1);
    instrBuilder.PushBackInstruction(bblock,
        instrBuilder.CreateAddi(ir::OperandType::U64, vdest, vreg, 1U),
        instrBuilder.CreateMul(ir::OperandType::I64, vdest, vreg, vreg));

    // auto *graph = irBuilder.GetGraph();
    return 0;
}
