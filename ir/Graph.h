#ifndef JIT_AOT_COMPILERS_COURSE_GRAPH_H_
#define JIT_AOT_COMPILERS_COURSE_GRAPH_H_

#include "AnalysisValidityManager.h"
#include <algorithm>
#include "AllocatorUtils.h"
#include "BasicBlock.h"
#include "macros.h"
#include "marker/marker.h"
#include <ranges>


namespace ir {
class CompilerBase;
class InstructionBuilder;
class Loop;

class Graph : public MarkerManager, public AnalysisValidityManager {
public:
    using IdType = FunctionId;

    Graph(CompilerBase *compiler, std::pmr::memory_resource *mem, InstructionBuilder *instrBuilder)
        : compiler(compiler),
          firstBlock(nullptr),
          lastBlock(nullptr),
          bblocks(mem),
          rpoBlocks(mem),
          loopTreeRoot(nullptr),
          instrBuilder(instrBuilder),
          memResource(mem)
    {
        ASSERT(compiler);
        ASSERT(memResource);
        ASSERT(instrBuilder);
    }
    NO_COPY_SEMANTIC(Graph);
    NO_MOVE_SEMANTIC(Graph);
    virtual DEFAULT_DTOR(Graph);

    IdType GetId() const {
        return id;
    }
    void SetId(IdType newId) {
        id = newId;
    }

    // TODO: may rework to get rid of `CompilerBase*` field as a bad design desition
    CompilerBase *GetCompiler() {
        return compiler;
    }

    std::pmr::memory_resource *GetMemoryResource() const {
        return memResource;
    }
    template <typename T, typename... ArgsT>
    [[nodiscard]] T *New(ArgsT&&... args) const {
        return utils::template New<T>(GetMemoryResource(), std::forward<ArgsT>(args)...);
    }

    BasicBlock *GetFirstBasicBlock() {
        return firstBlock;
    }
    const BasicBlock *GetFirstBasicBlock() const {
        return firstBlock;
    }
    BasicBlock *GetLastBasicBlock() {
        return lastBlock;
    }
    const BasicBlock *GetLastBasicBlock() const {
        return lastBlock;
    }
    size_t GetBasicBlocksCount() const {
        return bblocks.size() - unlinkedInstructionsCounter;
    }
    bool IsEmpty() const {
        return GetBasicBlocksCount() == 0;
    }

    auto GetRPO() const {
        ASSERT(IsAnalysisValid(AnalysisFlag::RPO));
        return std::span{rpoBlocks};
    }
    void SetRPO(std::pmr::vector<BasicBlock *> &&rpo) {
        rpoBlocks = std::move(rpo);
    }

    Loop *GetLoopTree() {
        return loopTreeRoot;
    }
    const Loop *GetLoopTree() const {
        return loopTreeRoot;
    }

    InstructionBuilder *GetInstructionBuilder() {
        return instrBuilder;
    }

    void SetFirstBasicBlock(BasicBlock *bblock) {
        firstBlock = bblock;
    }
    // Use this method carefully due to special meaning of last basic block
    void SetLastBasicBlock(BasicBlock *bblock) {
        lastBlock = bblock;
    }
    void SetLoopTree(Loop *loop) {
        loopTreeRoot = loop;
    }

    // assume either no side-effects are applied on basic blocks or user knows what he does
    template <typename FunctionType>
    requires UnaryFunctionType<FunctionType, BasicBlock *, void>
    void ForEachBasicBlock(FunctionType function) {
        auto nonNullPredicate = [](BasicBlock *bblock) { return bblock != nullptr; };
        std::ranges::for_each(
            std::views::all(bblocks) | std::views::filter(nonNullPredicate),
            function);
    }
    template <typename FunctionType>
    requires UnaryFunctionType<FunctionType, const BasicBlock *, void>
    void ForEachBasicBlock(FunctionType function) const {
        auto nonNullPredicate = [](const BasicBlock *bblock) { return bblock != nullptr; };
        std::ranges::for_each(
            std::views::all(bblocks) | std::views::filter(nonNullPredicate),
            function);
    }

    // O(NumberOfBasicBlocks) complexity
    size_t CountInstructions() const;

    BasicBlock *CreateEmptyBasicBlock(bool isTerminal = false);
    void ConnectBasicBlocks(BasicBlock *lhs, BasicBlock *rhs);
    void AddBasicBlock(BasicBlock *bblock);
    void AddBasicBlockBefore(BasicBlock *before, BasicBlock *bblock);
    void UnlinkBasicBlock(BasicBlock *bblock);
    void RemoveUnlinkedBlocks();

public:
    static constexpr IdType INVALID_ID = static_cast<IdType>(-1);

protected:
    void removePredecessors(BasicBlock *bblock);
    void removeSuccessors(BasicBlock *bblock);
    void unlinkBasicBlockImpl(BasicBlock *bblock);

private:
    IdType id = INVALID_ID;

    CompilerBase *compiler;

    BasicBlock *firstBlock;
    // last basic block collects all control flow exits from the graph
    BasicBlock *lastBlock;
    std::pmr::vector<BasicBlock *> bblocks;
    size_t unlinkedInstructionsCounter = 0;

    std::pmr::vector<BasicBlock *> rpoBlocks;

    Loop *loopTreeRoot;

    InstructionBuilder *instrBuilder;

    mutable std::pmr::memory_resource *memResource;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_GRAPH_H_
