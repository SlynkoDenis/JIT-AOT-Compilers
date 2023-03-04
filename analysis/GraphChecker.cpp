#include "GraphChecker.h"


namespace ir {
/* static */
void GraphChecker::VerifyControlAndDataFlowGraphs(const BasicBlock *bblock) {
    // TODO: implement more complete checks, e.g. on Loop & DomTree info
    ASSERT(bblock != nullptr);
    const InstructionBase *instr = bblock->GetFirstPhiInstruction();
    instr = instr ? instr : bblock->GetFirstInstruction();
    if (!instr) {
        ASSERT(bblock->GetLastPhiInstruction() == nullptr);
        ASSERT(bblock->GetLastInstruction() == nullptr);
        return;
    }
    size_t counter = 0;
    while (instr) {
        if (bblock->GetFirstPhiInstruction()) {
            if (instr != bblock->GetFirstPhiInstruction()) {
                ASSERT(instr->GetPrevInstruction() != nullptr);
            }
        } else if (instr != bblock->GetFirstInstruction()) {
            ASSERT(instr->GetPrevInstruction() != nullptr);
        }

        if (instr == bblock->GetLastInstruction()
            || (bblock->GetLastInstruction() == nullptr
                && instr == bblock->GetLastPhiInstruction()))
        {
            ASSERT(instr->GetNextInstruction() == nullptr);
        } else {
            ASSERT(instr->GetNextInstruction() != nullptr);
        }

        if (instr->HasInputs()) {
            auto *typed = static_cast<const InputsInstruction *>(instr);
            bool found = false;
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx; ++i) {
                auto currUsers = typed->GetInput(i)->GetUsers();
                auto iter = std::find(currUsers.begin(), currUsers.end(), instr);
                if (iter != currUsers.end()) {
                    found = true;
                    break;
                }
            }
            ASSERT(found == true);
        }

        auto inputUsers = instr->GetUsers();
        for (auto &&user : inputUsers) {
            ASSERT(user->HasInputs() == true);
            auto *typed = static_cast<const InputsInstruction *>(user);
            bool found = false;
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx; ++i) {
                if (typed->GetInput(i) == instr) {
                    found = true;
                    break;
                }
            }
            ASSERT(found == true);
        }

        ++counter;
        instr = instr->GetNextInstruction();
    }
    ASSERT(bblock->GetSize() == counter);
}
}   // namespace ir
