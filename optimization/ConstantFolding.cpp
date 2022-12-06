#include "ConstantFolding.h"
#include "InstructionBuilder.h"


namespace ir {
bool ConstantFolding::ProcessAND(BinaryRegInstruction *instr) {
    ASSERT((instr) && (instr->GetOpcode() == Opcode::AND));
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst() && input2->IsConst()) {
        auto value = asConst(input1.GetInstruction())->GetValue() &\
            asConst(input2.GetInstruction())->GetValue();
        auto *newInstr = getInstructionBuilder(instr)->CreateCONST(instr->GetType(), value);

        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetBasicBlock()->ReplaceInstruction(instr, newInstr);

        return true;
    }
    return false;
}

bool ConstantFolding::ProcessSRA(BinaryRegInstruction *instr) {
    ASSERT((instr) && (instr->GetOpcode() == Opcode::SRA));
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst() && input2->IsConst()) {
        auto value = ToSigned(asConst(input1.GetInstruction())->GetValue(), instr->GetType()) >>\
            asConst(input2.GetInstruction())->GetValue();
        auto *newInstr = getInstructionBuilder(instr)->CreateCONST(instr->GetType(), value);

        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetBasicBlock()->ReplaceInstruction(instr, newInstr);

        return true;
    }
    return false;
}

bool ConstantFolding::ProcessSUB(BinaryRegInstruction *instr) {
    ASSERT((instr) && (instr->GetOpcode() == Opcode::SUB));
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst() && input2->IsConst()) {
        auto value = asConst(input1.GetInstruction())->GetValue() -\
            asConst(input2.GetInstruction())->GetValue();
        auto *newInstr = getInstructionBuilder(instr)->CreateCONST(instr->GetType(), value);

        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetBasicBlock()->ReplaceInstruction(instr, newInstr);

        return true;
    }
    return false;
}

/* static */
ConstantInstruction *ConstantFolding::asConst(InstructionBase *instr) {
    ASSERT((instr) && instr->IsConst());
    return static_cast<ConstantInstruction *>(instr);
}

/* static */
InstructionBuilder *ConstantFolding::getInstructionBuilder(InstructionBase *instr) {
    return instr->GetBasicBlock()->GetGraph()->GetInstructionBuilder();
}
}   // namespace ir
