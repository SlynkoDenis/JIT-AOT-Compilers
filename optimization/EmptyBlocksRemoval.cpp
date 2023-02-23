#include "EmptyBlocksRemoval.h"
#include "Traversals.h"


namespace ir {
/* static */
void EmptyBlocksRemoval::RemoveIfEmpty(BasicBlock *bblock) {
    ASSERT(bblock);
    if (!bblock->IsEmpty() || bblock->IsLastInGraph()) {
        return;
    }
    if (bblock->HasNoPredecessors() && !bblock->HasNoSuccessors()) {
        ASSERT(bblock->IsFirstInGraph() && bblock->GetSuccessors().size() == 1);
        auto *succ = bblock->GetSuccessors()[0];
        ASSERT(succ->GetPredecessors().size() == 1 && succ->GetPredecessors()[0] == bblock);
        succ->GetPredecessors().clear();
        bblock->GetGraph()->SetFirstBasicBlock(succ);
    } else {
        BasicBlock *successorToSet = nullptr;
        if (!bblock->HasNoSuccessors()) {
            auto succs = bblock->GetSuccessors();
            ASSERT(succs.size() == 1);
            successorToSet = succs[0];
        }
        for (auto *pred : bblock->GetPredecessors()) {
            ASSERT(pred);
            auto succs = pred->GetSuccessors();
            ASSERT(!succs.empty() && succs.size() <= 2);

            if (successorToSet == nullptr) {
                if (succs.size() == 2) {
                    // branch can be removed, as it always taken / not taken
                    auto *jcmp = pred->GetLastInstruction();
                    ASSERT(jcmp && jcmp->IsBranch());
                    auto *cmp = jcmp->GetPrevInstruction();
                    ASSERT(cmp && cmp->GetOpcode() == Opcode::CMP);
                    pred->UnlinkInstruction(jcmp);
                    pred->UnlinkInstruction(cmp);
                }
                pred->RemoveSuccessor(bblock);
            } else {
                pred->ReplaceSuccessor(bblock, successorToSet);
            }
        }
    }
    bblock->GetPredecessors().clear();
    bblock->GetSuccessors().clear();
    bblock->GetGraph()->UnlinkBasicBlock(bblock);
}
}   // namespace ir
