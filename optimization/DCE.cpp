#include "DCE.h"
#include "Traversals.h"


namespace ir {
void DCEPass::Run() {
    aliveMarker = graph->GetNewMarker();

    auto rpoTraversal = RPO(graph);
    for (auto &bblock : rpoTraversal) {
        for (auto *instr : *bblock) {
            if (instructionHasSideEffects(instr)) {
                markAlive(instr);
            }
        }
    }

    for (auto &bblock : rpoTraversal) {
        for (auto *instr : *bblock) {
            if (!instr->IsMarkerSet(aliveMarker)) {
                markDead(instr);
            }
        }
    }

    removeDead();
}

void DCEPass::markAlive(InstructionBase *instr) {
    ASSERT(instr);
    dumper->Dump("Marking live instruction ", instr->GetId(), ' ', instr->GetOpcodeName());
    auto wasSet = instr->SetMarker(aliveMarker);
    if (instr->HasInputs() && wasSet) {
        auto *inputInstr = static_cast<InputsInstruction*>(instr);
        for (size_t i = 0, end = inputInstr->GetInputsCount(); i < end; ++i) {
            markAlive(inputInstr->GetInput(i).GetInstruction());
        }
    }
}

void DCEPass::markDead(InstructionBase *instr) {
    ASSERT(instr);
    dumper->Dump("Removing dead instruction ", instr->GetId(), ' ', instr->GetOpcodeName());
    deadInstrs.push_back(instr);
    // TODO: handle PHI case?
}

void DCEPass::removeDead() {
    for (auto *instr : deadInstrs) {
        instr->GetBasicBlock()->UnlinkInstruction(instr);
    }
    deadInstrs.clear();
}

/* static */
bool DCEPass::instructionHasSideEffects(InstructionBase *instr) {
    ASSERT(instr);
    return utils::to_underlying(instr->GetOpcode()) <= utils::to_underlying(Opcode::RETVOID);
}
}   // namespace ir
