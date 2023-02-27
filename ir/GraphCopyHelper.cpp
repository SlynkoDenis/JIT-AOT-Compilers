#include "GraphCopyHelper.h"


namespace ir {
/* static */
Graph *GraphCopyHelper::CreateCopy(const Graph *source, Graph *copyTarget) {
    ASSERT((source) && (copyTarget) && copyTarget->IsEmpty());
    GraphCopyHelper helper(source, copyTarget);
    helper.dfoCopy(source->GetFirstBasicBlock());
    ASSERT(helper.target->GetBasicBlocksCount() == helper.source->GetBasicBlocksCount());
    helper.fixDFG();
    return copyTarget;
}

void GraphCopyHelper::dfoCopy(const BasicBlock *currentBBlock) {
    ASSERT((currentBBlock) && !visited.contains(currentBBlock->GetId()));

    auto *bblockCopy = currentBBlock->Copy(target, instrsTranslation);
    if (currentBBlock == source->GetFirstBasicBlock()) {
        target->SetFirstBasicBlock(bblockCopy);
    }
    if (currentBBlock == source->GetLastBasicBlock()) {
        target->SetLastBasicBlock(bblockCopy);
    }
    visited.insert({currentBBlock->GetId(), bblockCopy});

    for (const auto *succ : currentBBlock->GetSuccessors()) {
        auto succCopyIter = visited.find(succ->GetId());
        if (succCopyIter != visited.end()) {
            // basic block was already visited
            target->ConnectBasicBlocks(bblockCopy, succCopyIter->second);
        } else {
            // visit basic block and attach the created copy with its predecessor
            dfoCopy(succ);
            target->ConnectBasicBlocks(bblockCopy, visited.at(succ->GetId()));
        }
    }
}

void GraphCopyHelper::fixDFG() {
    ASSERT(target->CountInstructions() == instrsTranslation.size());
    auto *translation = &instrsTranslation;

    target->ForEachBasicBlock([translation](BasicBlock *bblock) {
        ASSERT(bblock);
        std::for_each(bblock->begin(), bblock->end(), [translation](InstructionBase *instr) {
            // set correct inputs
            if (instr->HasInputs()) {
                auto *withInputs = static_cast<InputsInstruction *>(instr);
                for (size_t i = 0, end = withInputs->GetInputsCount(); i < end; ++i) {
                    auto &input = withInputs->GetInput(i);
                    withInputs->SetInput(translation->at(input->GetId()), i);
                }
            }

            // set correct users
            std::pmr::vector<InstructionBase *> newUsers(translation->get_allocator());
            newUsers.reserve(instr->UsersCount());
            for (auto *user : instr->GetUsers()) {
                newUsers.push_back(translation->at(user->GetId()));
            }
            instr->SetNewUsers(std::move(newUsers));
        });
    });
}
}   // namespace ir
