#include "Compiler.h"
#include "graph_copy/GraphCopyHelper.h"
#include "Loop.h"


// TODO: move builders & compiler into a dedicated ir's subdirectory
namespace ir {
Graph *Compiler::CreateNewGraph(InstructionBuilder *instrBuilder) {
    auto *graph = utils::template New<Graph>(&memResource, this, &memResource, instrBuilder);
    graph->SetId(functionsGraphs.size());
    functionsGraphs.push_back(graph);
    return graph;
}

// Depth first ordered graph copy algorithm implementation.
Graph *Compiler::CopyGraph(const Graph *source, InstructionBuilder *instrBuilder) {
    ASSERT((source) && (instrBuilder));
    return GraphCopyHelper::CreateCopy(source, CreateNewGraph(instrBuilder));
}

Graph *Compiler::Optimize(Graph *graph) {
    // TODO: run optimizations here
    return graph;
}
}   // namespace ir
