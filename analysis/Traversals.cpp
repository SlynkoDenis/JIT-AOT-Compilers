#include <functional>
#include <set>
#include "Traversals.h"


namespace ir {
void DFO::DoTraverse(Graph *graph, DFO::TraversalCallbackType callback) {
    ASSERT(graph);
    if (graph->IsEmpty()) {
        return;
    }

    visited.clear();
    doTraverse(graph->GetFirstBasicBlock(), callback);
    ASSERT(visited.size() == graph->GetBasicBlocksCount());
}

void DFO::doTraverse(BasicBlock *bblock, DFO::TraversalCallbackType callback) {
    ASSERT(bblock);
    visited.insert(bblock->GetId());
    for (auto *succ : bblock->GetSuccessors()) {
        if (!visited.contains(succ->GetId())) {
            doTraverse(succ, callback);
        }
    }
    callback(bblock);
}

std::vector<BasicBlock *> RPO(Graph *graph) {
    ASSERT(graph);
    std::vector<BasicBlock *> result;
    if (graph->IsEmpty()) {
        return result;
    }

    result.reserve(graph->GetBasicBlocksCount());
    DFO::Traverse(graph, [&result](BasicBlock *bblock){ result.push_back(bblock); });
    ASSERT(result.size() == graph->GetBasicBlocksCount());
    std::reverse(result.begin(), result.end());
    return result;
}
}   // namespace ir
