#include "Graph.h"
#include "Traversals.h"


namespace ir {
void DFO::DoTraverse(Graph *graph, DFO::TraversalCallbackType callback) {
    ASSERT(graph);
    if (graph->IsEmpty()) {
        return;
    }

    if (!visited) {
        auto *allocator = graph->GetAllocator();
        visited = allocator->template New<utils::memory::ArenaSet<size_t>>(allocator->ToSTL());
    } else {
        visited->clear();
    }
    doTraverse(graph->GetFirstBasicBlock(), callback);
    ASSERT(visited->size() == graph->GetBasicBlocksCount());
}

void DFO::doTraverse(BasicBlock *bblock, DFO::TraversalCallbackType callback) {
    ASSERT(bblock);
    visited->insert(bblock->GetId());
    for (auto *succ : bblock->GetSuccessors()) {
        if (!visited->contains(succ->GetId())) {
            doTraverse(succ, callback);
        }
    }
    callback(bblock);
}

utils::memory::ArenaVector<BasicBlock *> RPO(Graph *graph) {
    ASSERT(graph);
    utils::memory::ArenaVector<BasicBlock *> result(graph->GetAllocator()->ToSTL());
    if (graph->IsEmpty()) {
        return result;
    }

    result.reserve(graph->GetBasicBlocksCount());
    DFO::Traverse(graph, [&result](BasicBlock *bblock){ result.push_back(bblock); });
    ASSERT(result.size() == graph->GetBasicBlocksCount());
    std::reverse(result.begin(), result.end());
    return result;
}

void DumpGrahpRPO(Graph *graph) {
    auto rpoBBlocks = RPO(graph);
    std::cout << "======================================\n";
    for (auto bblock : rpoBBlocks) {
        std::cout << "\tBB #" << bblock->GetId() << "\npreds: < ";
        for (const auto &pred : bblock->GetPredecessors()) {
            std::cout << pred->GetId() << " ";
        }
        std::cout << ">\nsuccs: < ";
        for (const auto &succ : bblock->GetSuccessors()) {
            std::cout << succ->GetId() << " ";
        }
        std::cout << ">\n";
    }
    std::cout << "======================================" << std::endl;
}
}   // namespace ir
