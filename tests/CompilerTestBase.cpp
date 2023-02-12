#include "CompilerTestBase.h"
#include "Traversals.h"


namespace ir::tests {
/* static */
void CompilerTestBase::VerifyControlAndDataFlowGraphs(const Graph *graph) {
    graph->ForEachBasicBlock([](const BasicBlock *bblock) {
        if (!bblock->IsLastInGraph()) {
            VerifyControlAndDataFlowGraphs(bblock);
        }
    });
}

/* static */
void CompilerTestBase::VerifyControlAndDataFlowGraphs(const BasicBlock *bblock) {
    ASSERT_NE(bblock, nullptr);
    const InstructionBase *instr = bblock->GetFirstPhiInstruction();
    instr = instr ? instr : bblock->GetFirstInstruction();
    if (!instr) {
        return;
    }
    size_t counter = 0;
    while (instr) {
        if (instr != bblock->GetFirstPhiInstruction() && instr != bblock->GetFirstInstruction()) {
            ASSERT_NE(instr->GetPrevInstruction(), nullptr);
        }
        if (instr != bblock->GetLastInstruction()) {
            ASSERT_NE(instr->GetNextInstruction(), nullptr);
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
            ASSERT_TRUE(found);
        }

        auto inputUsers = instr->GetUsers();
        for (auto &&user : inputUsers) {
            ASSERT_TRUE(user->HasInputs());
            auto *typed = static_cast<const InputsInstruction *>(user);
            bool found = false;
            for (size_t i = 0, end_idx = typed->GetInputsCount(); i < end_idx; ++i) {
                if (typed->GetInput(i) == instr) {
                    found = true;
                    break;
                }
            }
            ASSERT_TRUE(found);
        }

        if (instr->GetNextInstruction() == nullptr) {
            ASSERT_EQ(instr, bblock->GetLastInstruction());
        }
        ++counter;
        instr = instr->GetNextInstruction();
    }
    ASSERT_EQ(bblock->GetSize(), counter);
}
}   // namespace ir::tests
