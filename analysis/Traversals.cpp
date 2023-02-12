#include "Graph.h"
#include "Traversals.h"
#include "dumper/StdOutputDumper.h"


namespace ir {
void dumpBasicBlock(const BasicBlock *bblock) {
    ASSERT(bblock);
    std::cout << "=== BB #" << bblock->GetId() << " (" << bblock->GetSize();
    std::cout << " instructions)\npreds: < ";
    for (const auto &pred : bblock->GetPredecessors()) {
        std::cout << pred->GetId() << " ";
    }
    std::cout << ">\nsuccs: < ";
    for (const auto &succ : bblock->GetSuccessors()) {
        std::cout << succ->GetId() << " ";
    }
    std::cout << ">\n";

    auto *dumper = utils::dumper::EventDumper::AddDumper<utils::dumper::StdOutputDumper>(
        bblock->GetGraph()->GetAllocator(),
        "StdOutputDumper").second;
    for (auto *instr : *bblock) {
        instr->Dump(dumper);
        std::cout << '\n';
    }
    std::cout << '\n';
}

void DumpGraphRPO(const Graph *graph) {
    auto rpoBBlocks = RPO(graph);
    std::cout << "======================================\n";
    for (auto bblock : rpoBBlocks) {
        dumpBasicBlock(bblock);
    }
    std::cout << "======================================" << std::endl;
}

void DumpGraphDFO(const Graph *graph) {
    std::cout << "======================================\n";
    DFO::Traverse(graph, dumpBasicBlock);
    std::cout << "======================================" << std::endl;
}
}   // namespace ir
