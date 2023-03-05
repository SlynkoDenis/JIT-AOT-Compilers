#include "Compiler.h"
#include "Traversals.h"


int main() {
    ir::Compiler compiler;
    auto *graph = compiler.CreateNewGraph();
    graph = compiler.Optimize(graph);
    ir::DumpGraphRPO(graph);
    return 0;
}
