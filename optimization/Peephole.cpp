#include "InstructionBuilder.h"
#include <limits>
#include "Peephole.h"
#include "Traversals.h"


namespace ir {
void PeepholePass::Run() {
    for (auto &bblock : RPO(graph)) {
        for (auto *instr = bblock->GetFirstInstruction(); instr != nullptr;
             instr = instr->GetNextInstruction()) {
            switch (instr->GetOpcode()) {
            case Opcode::AND:
                ProcessAND(instr);
                break;
            case Opcode::SRA:
                ProcessSRA(instr);
                break;
            case Opcode::SUB:
                ProcessSUB(instr);
                break;
            default:
                break;
            }
        }
    }
}

void PeepholePass::ProcessAND(InstructionBase *instr) {
    ASSERT(instr->GetOpcode() == Opcode::AND);
    BinaryRegInstruction *typed = static_cast<BinaryRegInstruction *>(instr);

    if (foldingPass.ProcessAND(typed)) {
        dumper->Dump("Folded AND instruction");
        return;
    }

    if (tryANDRepeatedArgs(typed)) {
        return;
    }
    if (tryANDAfterNOT(typed)) {
        return;
    }
    // TODO: replace according to IDs order
    auto input1 = typed->GetInput(0);
    auto input2 = typed->GetInput(1);
    if (tryConstantAND(typed, input1, input2)) {
        return;
    }
    if (tryConstantAND(typed, input2, input1)) {
        return;
    }
}

void PeepholePass::ProcessSRA(InstructionBase *instr) {
    ASSERT(instr->GetOpcode() == Opcode::SRA);
    BinaryRegInstruction *typed = static_cast<BinaryRegInstruction *>(instr);

    if (foldingPass.ProcessSRA(typed)) {
        dumper->Dump("Folded SRA instruction");
        return;
    }

    if (trySRAZero(typed)) {
        return;
    }
    // if (trySequencedSRA(typed)) {
    //     return;
    // }
}

void PeepholePass::ProcessSUB(InstructionBase *instr) {
    ASSERT(instr->GetOpcode() == Opcode::SUB);
    BinaryRegInstruction *typed = static_cast<BinaryRegInstruction *>(instr);

    if (foldingPass.ProcessSUB(typed)) {
        dumper->Dump("Folded SUB instruction");
        return;
    }

    if (trySUBRepeatedArgs(typed)) {
        return;
    }
    if (trySUBZero(typed)) {
        return;
    }
    if (trySUBAfterADD(typed)) {
        return;
    }
    if (trySUBAfterNEG(typed, typed->GetInput(1), typed->GetInput(0))) {
        return;
    }
}

bool PeepholePass::tryConstantAND(BinaryRegInstruction *instr, Input checked, Input second) {
    if (checked->IsConst()) {
        ASSERT(checked->GetType() == instr->GetType());
        auto *inputInstr = static_cast<ConstantInstruction *>(checked.GetInstruction());
        if (inputInstr->GetValue() == static_cast<ConstantInstruction::Type>(0)) {
            // case: v1 = v0 & 0 -> v1 = 0
            replaceWithoutNewInstr(instr, inputInstr);
            dumper->Dump("One of the AND inputs is constant zero");
            return true;
        } else if (inputInstr->GetValue() == GetMaxValue(instr->GetType())) {
            // case: v1 = v0 & (0 - 1) -> v1 = v0
            replaceWithoutNewInstr(instr, second.GetInstruction());
            dumper->Dump("One of the AND inputs is logical one");
            return true;
        }
    }
    return false;
}

bool PeepholePass::tryANDAfterNOT(BinaryRegInstruction *instr) {
    // case:
    // v2 = ~v0
    // v3 = ~v1
    // v4 = v2 & v3
    // in case of single use of v2 and v3 is replaced with
    // v5 = v0 | v1
    // v4 = ~v5
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->GetOpcode() == Opcode::NOT
            && input2->GetOpcode() == Opcode::NOT
            && input1->UsersCount() == 1
            && input2->UsersCount() == 1) {
        auto not1Arg = static_cast<UnaryRegInstruction *>(input1.GetInstruction())->GetInput(0);
        auto not2Arg = static_cast<UnaryRegInstruction *>(input2.GetInstruction())->GetInput(0);
        auto *orInstr = graph->GetInstructionBuilder()->CreateOR(instr->GetType(), not1Arg, not2Arg);

        not1Arg->ReplaceUser(input1.GetInstruction(), orInstr);
        not2Arg->ReplaceUser(input2.GetInstruction(), orInstr);
        auto *bblock = instr->GetBasicBlock();
        bblock->UnlinkInstruction(input1.GetInstruction());
        bblock->UnlinkInstruction(input2.GetInstruction());

        instr->GetBasicBlock()->InsertBefore(instr, orInstr);

        auto *notInstr = graph->GetInstructionBuilder()->CreateNOT(instr->GetType(), orInstr);
        orInstr->AddUser(notInstr);
        // input1 and input2 instructions are later removed on DCE
        input1->RemoveUser(instr);
        input2->RemoveUser(instr);
        instr->GetBasicBlock()->ReplaceInstruction(instr, notInstr);

        dumper->Dump("Applied AND: 'v2 = ~v0 & ~v1' -> 'v2 = ~(v0 | v1)' peephole");
        return true;
    }
    return false;
}

bool PeepholePass::tryANDRepeatedArgs(BinaryRegInstruction *instr) {
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1 == input2) {
        // case: v1 = v0 & v0 -> v1 = v0
        replaceWithoutNewInstr(instr, input1.GetInstruction());
        dumper->Dump("Applied AND: 'v1 = v0 & v0' -> 'v1 = v0' peephole");
        return true;
    }
    return false;
}

bool PeepholePass::trySequencedSRA(BinaryRegInstruction *instr) {
    // TODO: enable this peephole after specifying overflow behaviour
    // case:
    // v3 = v0 >> v1
    // v4 = v3 >> v2
    // replaced with
    // v5 = v1 + v2
    // v4 = v0 >> v5
    auto *base = instr->GetInput(0).GetInstruction();
    if (base->GetOpcode() == Opcode::SRA) {
        auto *typed = static_cast<BinaryRegInstruction *>(base);
        auto base2 = typed->GetInput(0);

        auto offset1 = instr->GetInput(1);
        auto offset2 = typed->GetInput(1);
        auto *addInstr = graph->GetInstructionBuilder()->CreateADD(instr->GetType(), offset1,
                                                                   offset2);
        offset1->ReplaceUser(instr, addInstr);
        offset2->AddUser(addInstr);
        instr->GetBasicBlock()->InsertBefore(instr, addInstr);

        base->RemoveUser(instr);
        instr->ReplaceInput(instr->GetInput(0), base2);
        instr->ReplaceInput(offset1, addInstr);
        base2->AddUser(instr);
        addInstr->AddUser(instr);

        dumper->Dump("Applied SRA -> SRA peephole");
        return true;
    } else if (base->GetOpcode() == Opcode::SRAI) {
        auto *typed = static_cast<BinaryImmInstruction *>(base);
        auto base2 = typed->GetInput(0);

        auto offset1 = instr->GetInput(1);
        auto offset2 = typed->GetValue();
        auto *addInstr = graph->GetInstructionBuilder()->CreateADDI(instr->GetType(), offset1,
                                                                    offset2);
        offset1->ReplaceUser(instr, addInstr);
        instr->GetBasicBlock()->InsertBefore(instr, addInstr);

        base->RemoveUser(instr);
        instr->ReplaceInput(instr->GetInput(0), base2);
        instr->ReplaceInput(offset1, addInstr);
        base2->AddUser(instr);
        addInstr->AddUser(instr);

        dumper->Dump("Applied SRAI -> SRA peephole");
        return true;
    }
    return false;
}

bool PeepholePass::trySRAZero(BinaryRegInstruction *instr) {
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst()) {
        auto *typed = static_cast<ConstantInstruction *>(input1.GetInstruction());
        if (typed->GetValue() == 0) {
            replaceWithoutNewInstr(instr, typed);
            dumper->Dump("Applied '0 >>> v' peephole");
            return true;
        }
    }
    if (input2->IsConst()) {
        auto *typed = static_cast<ConstantInstruction *>(input2.GetInstruction());
        if (typed->GetValue() == 0) {
            replaceWithoutNewInstr(instr, input1.GetInstruction());
            dumper->Dump("Applied 'v >>> 0' peephole");
            return true;
        }
    }
    return false;
}

bool PeepholePass::trySUBAfterNEG(BinaryRegInstruction *instr, Input checked, Input second) {
    // case:
    // v2 = -v0
    // v3 = v1 - v2
    // replaced with
    // v3 = v1 + v0
    if (checked->GetOpcode() == Opcode::NEG) {
        auto *typedInput = static_cast<UnaryRegInstruction *>(checked.GetInstruction());

        auto positiveValueInput = typedInput->GetInput();
        // TODO: replace according to IDs order
        auto *newInstr = graph->GetInstructionBuilder()->CreateADD(instr->GetType(), positiveValueInput, second);
        positiveValueInput->AddUser(newInstr);

        instr->GetBasicBlock()->ReplaceInstruction(instr, newInstr);
        checked->RemoveUser(instr);
        second->ReplaceUser(instr, newInstr);

        dumper->Dump("Applied NEG -> SUB peephole");
        return true;
    }
    return false;
}

bool PeepholePass::trySUBAfterADD(BinaryRegInstruction *instr) {
    auto instrInput1 = instr->GetInput(0);
    auto instrInput2 = instr->GetInput(1);
    // case:
    // v2 = v1 + v0
    // v3 = v2 - v0
    // replaced with
    // v3 = v1
    if (instrInput1->GetOpcode() == Opcode::ADD) {
        auto *typed = static_cast<BinaryRegInstruction *>(instrInput1.GetInstruction());
        auto input1 = typed->GetInput(0);
        auto input2 = typed->GetInput(1);

        InstructionBase *newInstr = nullptr;
        if (instrInput2 == input1) {
            newInstr = input2.GetInstruction();
        } else if (instrInput2 == input2) {
            newInstr = input1.GetInstruction();
        }
        if (newInstr) {
            replaceWithoutNewInstr(instr, newInstr);
            dumper->Dump("Applied ADD -> SUB peephole");
            return true;
        }
    } else if (instrInput1->GetOpcode() == Opcode::ADDI) {
        auto *typed = static_cast<BinaryImmInstruction *>(instrInput1.GetInstruction());
        auto input1 = typed->GetInput(0);
        if (input1 == instrInput2) {
            auto *constInstr = graph->GetInstructionBuilder()->CreateCONST(instr->GetType(), typed->GetValue());
            typed->RemoveUser(instr);
            instr->GetBasicBlock()->ReplaceInstruction(instr, constInstr);
            dumper->Dump("Applied ADDI -> SUB peephole");
            return true;
        }
    }

    // case:
    // v2 = v1 + v0
    // v3 = v0 - v2
    // replaced with
    // v3 = -v1
    if (instrInput2->GetOpcode() == Opcode::ADD) {
        auto *typed = static_cast<BinaryRegInstruction *>(instrInput2.GetInstruction());
        auto input1 = typed->GetInput(0);
        auto input2 = typed->GetInput(1);

        InstructionBase *userInstr = nullptr;
        if (instrInput1 == input1) {
            userInstr = input2.GetInstruction();
        } else if (instrInput1 == input2) {
            userInstr = input1.GetInstruction();
        }
        if (userInstr) {
            auto *negInstr = graph->GetInstructionBuilder()->CreateNEG(instr->GetType(), userInstr);
            userInstr->AddUser(negInstr);

            instrInput1->RemoveUser(instr);
            instrInput2->RemoveUser(instr);
            instr->GetBasicBlock()->ReplaceInstruction(instr, negInstr);
            dumper->Dump("Applied ADD -> SUB peephole");
            return true;
        }
    } else if (instrInput2->GetOpcode() == Opcode::ADDI) {
        auto *typed = static_cast<BinaryImmInstruction *>(instrInput2.GetInstruction());
        auto input1 = typed->GetInput(0);
        if (input1 == instrInput1) {
            auto *constInstr = graph->GetInstructionBuilder()->CreateCONST(instr->GetType(), -(typed->GetValue()));
            typed->RemoveUser(instr);
            instr->GetBasicBlock()->ReplaceInstruction(instr, constInstr);
            dumper->Dump("Applied ADDI -> SUB peephole");
            return true;
        }
    }
    return false;
}

bool PeepholePass::trySUBZero(BinaryRegInstruction *instr) {
    auto input1 = instr->GetInput(0);
    auto input2 = instr->GetInput(1);
    if (input1->IsConst()) {
        auto *typed = static_cast<ConstantInstruction *>(input1.GetInstruction());
        if (typed->GetValue() == 0) {
            auto *negInstr = graph->GetInstructionBuilder()->CreateNEG(input2->GetType(), input2);
            input1->RemoveUser(instr);
            input2->ReplaceUser(instr, negInstr);
            instr->GetBasicBlock()->ReplaceInstruction(instr, negInstr);
            dumper->Dump("Applied SUB: '0 - v' peephole");
            return true;
        }
    }
    if (input2->IsConst()) {
        auto *typed = static_cast<ConstantInstruction *>(input2.GetInstruction());
        if (typed->GetValue() == 0) {
            replaceWithoutNewInstr(instr, input1.GetInstruction());
            dumper->Dump("Applied SUB: 'v - 0' peephole");
            return true;
        }
    }
    return false;
}

bool PeepholePass::trySUBRepeatedArgs(BinaryRegInstruction *instr) {
    if (instr->GetInput(0) == instr->GetInput(1)) {
        // case: v1 = v0 - v0 -> v1 = 0
        auto *constZero = graph->GetInstructionBuilder()->CreateCONST(instr->GetType(), 0);
        instr->GetBasicBlock()->ReplaceInstruction(instr, constZero);
        dumper->Dump("Applied SUB: 'v1 = v0 - v0' -> 'v1 = 0' peephole");
        return true;
    }
    return false;
}

void PeepholePass::replaceWithoutNewInstr(BinaryRegInstruction *instr,
                                          InstructionBase *replacedInstr) {
    ASSERT(instr);
    instr->ReplaceInputInUsers(replacedInstr);
    instr->GetBasicBlock()->UnlinkInstruction(instr);

    // these instructions may be deleted later by DCE
    instr->GetInput(0)->RemoveUser(instr);
    instr->GetInput(1)->RemoveUser(instr);
}
}   // namespace ir
