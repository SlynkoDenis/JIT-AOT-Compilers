#ifndef JIT_AOT_COMPILERS_COURSE_COMPILER_H_
#define JIT_AOT_COMPILERS_COURSE_COMPILER_H_

#include "arena/ArenaAllocator.h"
#include "CompilerBase.h"
#include "InstructionBuilder.h"


namespace ir {
using namespace utils::memory;

class Compiler : public CompilerBase {
public:
    Compiler() : allocator(), functionsGraphs(allocator.ToSTL()) {}

    Graph *CreateNewGraph() override {
        auto *instrBuilder = allocator.template New<InstructionBuilder>(&allocator);
        return CreateNewGraph(instrBuilder);
    }
    Graph *CreateNewGraph(InstructionBuilder *instrBuilder);
    Graph *CopyGraph(const Graph *source, InstructionBuilder *instrBuilder) override;
    Graph *Optimize(Graph *graph) override {
        // TODO: run optimizations here
        return graph;
    }
    Graph *GetFunction(FunctionId functionId) override {
        if (functionId >= functionsGraphs.size()) {
            return nullptr;
        }
        return functionsGraphs[functionId];
    }
    bool DeleteFunctionGraph(FunctionId functionId) override {
        if (functionId >= functionsGraphs.size()) {
            return false;
        }
        functionsGraphs.erase(functionsGraphs.begin() + functionId);
        return true;
    }

private:
    ArenaAllocator allocator;

    ArenaVector<Graph *> functionsGraphs;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_COMPILER_H_
