#ifndef JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_
#define JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_

#include "arena/ArenaAllocator.h"
#include "BasicBlock.h"
#include <functional>
#include <set>
#include <vector>


namespace ir {
class Graph;

using TraversalCallbackType = std::function<void(BasicBlock *)>;
using ConstTraversalCallbackType = std::function<void(const BasicBlock *)>;

template <typename CallbackT>
concept ValidCallback = (std::is_same_v<CallbackT, TraversalCallbackType>
    || std::is_same_v<CallbackT, ConstTraversalCallbackType>);

template <typename GraphT>
concept GraphType = std::is_same_v<std::remove_cv_t<GraphT>, Graph>;

class DFO {
public:
    virtual DEFAULT_DTOR(DFO);

    static void Traverse(Graph *graph, TraversalCallbackType callback) {
        auto dfo = DFO();
        dfo.DoTraverse(graph, callback);
    }
    static void Traverse(const Graph *graph, ConstTraversalCallbackType callback) {
        auto dfo = DFO();
        dfo.DoTraverse(graph, callback);
    }

    template <GraphType GraphT, ValidCallback CallbackT>
    void DoTraverse(GraphT *graph, CallbackT callback)
    {
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

protected:
    template <typename BBlockT, ValidCallback CallbackT>
    void doTraverse(BBlockT *bblock, CallbackT callback)
    requires std::is_same_v<std::remove_cv_t<BBlockT>, BasicBlock>
    {
        ASSERT(bblock);
        visited->insert(bblock->GetId());
        for (auto *succ : bblock->GetSuccessors()) {
            if (!visited->contains(succ->GetId())) {
                doTraverse(succ, callback);
            }
        }
        callback(bblock);
    }

private:
    utils::memory::ArenaSet<size_t> *visited = nullptr;
};

// concepts and helpers
template <GraphType GraphT>
struct BasicBlockTypeHelper {
    using type = const BasicBlock;
};

template <>
struct BasicBlockTypeHelper<Graph> {
    using type = BasicBlock;
};

template <GraphType GraphT>
using BasickBlockType = typename BasicBlockTypeHelper<GraphT>::type;

template <GraphType GraphT>
utils::memory::ArenaVector<BasickBlockType<GraphT> *> RPO(GraphT *graph) {
    ASSERT(graph);
    utils::memory::ArenaVector<BasickBlockType<GraphT> *> result(graph->GetAllocator()->ToSTL());
    if (graph->IsEmpty()) {
        return result;
    }

    result.reserve(graph->GetBasicBlocksCount());
    DFO::Traverse(graph, [&result](BasickBlockType<GraphT> *bblock){ result.push_back(bblock); });
    ASSERT(result.size() == graph->GetBasicBlocksCount());
    std::reverse(result.begin(), result.end());
    return result;
}

void DumpGraphRPO(const Graph *graph);

void DumpGraphDFO(const Graph *graph);
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_
