#ifndef JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_
#define JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_

#include "BasicBlock.h"
#include <functional>
#include <set>
#include <vector>


namespace ir {
class Graph;

class DFO {
public:
    using TraversalCallbackType = std::function<void(BasicBlock *)>;

    virtual DEFAULT_DTOR(DFO);

    void DoTraverse(Graph *graph, TraversalCallbackType callback);
    static void Traverse(Graph *graph, TraversalCallbackType callback) {
        auto dfo = DFO();
        dfo.DoTraverse(graph, callback);
    }

protected:
    virtual void doTraverse(BasicBlock *bblock, TraversalCallbackType callback);

private:
    std::set<size_t> visited;
};

std::vector<BasicBlock *> RPO(Graph *graph);

void DumpGrahpRPO(Graph *graph);
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_
