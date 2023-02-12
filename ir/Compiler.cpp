#include "Compiler.h"
#include "GraphCopyHelper.h"
#include "Loop.h"


// TODO: move builders & compiler into a dedicated ir's subdirectory
namespace ir {
Graph *Compiler::CreateNewGraph(InstructionBuilder *instrBuilder) {
    auto *graph = allocator.template New<Graph>(this, &allocator, instrBuilder);
    graph->SetId(functionsGraphs.size());
    functionsGraphs.push_back(graph);
    return graph;
}

// Depth first ordered graph copy algorithm implementation.
Graph *Compiler::CopyGraph(const Graph *source, InstructionBuilder *instrBuilder) {
    ASSERT((source) && (instrBuilder));
    GraphCopyHelper helper(source);
    return helper.CreateCopy(CreateNewGraph(instrBuilder));
}

// defined here after full declaration of Compiler's methods
void copyInstruction(
    BasicBlock *targetBlock,
    const InstructionBase *orig,
    ArenaUnorderedMap<InstructionBase::IdType, InstructionBase *> *instrsTranslation)
{
    ASSERT((targetBlock) && (orig) && (instrsTranslation));

    auto it = instrsTranslation->find(orig->GetId());
    ASSERT(it == instrsTranslation->end());

    auto *copy = orig->Copy(targetBlock);
    targetBlock->PushBackInstruction(copy);
    instrsTranslation->insert(it, {orig->GetId(), copy});
}

BasicBlock *BasicBlock::Copy(
    Graph *targetGraph,
    ArenaUnorderedMap<InstructionBase::IdType, InstructionBase *> *instrsTranslation) const
{
    ASSERT(targetGraph);
    auto *result = targetGraph->CreateEmptyBasicBlock();

    const InstructionBase *instr = GetFirstPhiInstruction();
    if (!instr) {
        instr = GetFirstInstruction();
        if (!instr) {
            return result;
        }
    }
    for (auto *end = GetLastInstruction(); instr != end; instr = instr->GetNextInstruction()) {
        copyInstruction(result, instr, instrsTranslation);
    }
    // copy the last instruction
    copyInstruction(result, instr, instrsTranslation);

    ASSERT(result->GetFirstInstruction());
    ASSERT(result->GetLastInstruction());
    return result;
}

// TODO: write unit tests for this method
std::pair<BasicBlock *, BasicBlock *> BasicBlock::SplitAfterInstruction(InstructionBase *instr,
                                                                        bool connectAfterSplit) {
    ASSERT((instr) && instr->GetBasicBlock() == this);
    auto *nextInstr = instr->GetNextInstruction();
    ASSERT(nextInstr);

    auto *graph = GetGraph();
    auto *newBBlock = graph->CreateEmptyBasicBlock();

    // might leave unconnected, e.g. for further usage in inlining
    if (connectAfterSplit) {
        graph->ConnectBasicBlocks(this, newBBlock);
    }

    if (GetLoop()) {
        GetLoop()->AddBasicBlock(newBBlock);
        newBBlock->SetLoop(GetLoop());
    }
    for (auto *succ : GetSuccessors()) {
        succ->RemovePredecessor(this);
        graph->ConnectBasicBlocks(newBBlock, succ);
    }
    succs.clear();

    instr->SetNextInstruction(nullptr);
    nextInstr->SetPrevInstruction(nullptr);
    for (auto *iter = nextInstr; iter != nullptr; iter = iter->GetNextInstruction()) {
        iter->SetBasicBlock(newBBlock);
        newBBlock->instrsCount += 1;
    }
    instrsCount -= newBBlock->instrsCount;

    if (nextInstr->IsPhi()) {
        ASSERT(instr->IsPhi());
        newBBlock->firstPhi = static_cast<PhiInstruction *>(nextInstr);
        newBBlock->lastPhi = lastPhi;
        lastPhi = static_cast<PhiInstruction *>(instr);
        // can be done unconditionally
        newBBlock->firstInst = firstInst;
        newBBlock->lastInst = lastInst;
        firstInst = nullptr;
        lastInst = nullptr;
    } else {
        newBBlock->firstInst = nextInstr;
        newBBlock->lastInst = lastInst;
        lastInst = instr;
    }

    return {this, newBBlock};
}
}   // namespace ir
