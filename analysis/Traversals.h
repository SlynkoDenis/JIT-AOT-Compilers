#ifndef JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_
#define JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_

#include "BasicBlock.h"
#include <functional>
#include "PassBase.h"
#include <unordered_set>
#include <vector>


namespace ir {
class Graph;

template <typename CallbackT>
concept TraversalCallback = requires (CallbackT c, BasicBlock *b) {
    { c(b) } -> std::convertible_to<void>;
};

template <typename CallbackT>
concept ConstTraversalCallback = requires (CallbackT c, const BasicBlock *b) {
    { c(b) } -> std::convertible_to<void>;
};

template <typename CallbackT>
concept ValidCallback = TraversalCallback<CallbackT> || ConstTraversalCallback<CallbackT>;

template <typename GraphT>
concept GraphType = std::is_same_v<std::remove_cv_t<GraphT>, Graph>;

class DFO final {
public:
    template <GraphType GraphT, ValidCallback CallbackT>
    static void Traverse(GraphT *graph, CallbackT callback) {
        ASSERT(graph);
        if (graph->IsEmpty()) {
            return;
        }

        DFO helper(graph->GetMemoryResource());
        helper.doTraverse(graph->GetFirstBasicBlock(), callback);
        ASSERT(helper.visited.size() == graph->GetBasicBlocksCount());
    }

private:
    DFO(std::pmr::memory_resource *memResource) : visited(memResource) {}
    DEFAULT_COPY_SEMANTIC(DFO);
    DEFAULT_MOVE_SEMANTIC(DFO);
    DEFAULT_DTOR(DFO);

    template <typename BBlockT, ValidCallback CallbackT>
    void doTraverse(BBlockT *bblock, CallbackT callback)
    requires std::is_same_v<std::remove_cv_t<BBlockT>, BasicBlock>
    {
        ASSERT(bblock);
        visited.insert(bblock->GetId());
        for (auto *succ : bblock->GetSuccessors()) {
            if (!visited.contains(succ->GetId())) {
                doTraverse(succ, callback);
            }
        }
        callback(bblock);
    }

private:
    std::pmr::unordered_set<size_t> visited;
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

class RPO : public PassBase {
public:
    RPO(Graph *graph) : PassBase(graph) {}
    NO_COPY_SEMANTIC(RPO);
    NO_MOVE_SEMANTIC(RPO);
    ~RPO() noexcept override = default;

    void Run() override {
        graph->SetRPO(std::move(DoRPO(graph)));
    }

    template <GraphType GraphT>
    static std::pmr::vector<BasickBlockType<GraphT> *> DoRPO(GraphT *graph) {
        ASSERT(graph);
        std::pmr::vector<BasickBlockType<GraphT> *> result(graph->GetMemoryResource());
        if (graph->IsEmpty()) {
            return result;
        }

        result.reserve(graph->GetBasicBlocksCount());
        DFO::Traverse(graph, [&result](BasickBlockType<GraphT> *bblock){ result.push_back(bblock); });
        ASSERT(result.size() == graph->GetBasicBlocksCount());
        std::reverse(result.begin(), result.end());
        return result;
    }

public:
    static constexpr AnalysisFlag SET_FLAG = AnalysisFlag::RPO;
};

void DumpGraphRPO(const Graph *graph);

void DumpGraphDFO(const Graph *graph);
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_TRAVERSALS_H_
