#ifndef JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_
#define JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_

#include "Compiler.h"
#include "InstructionBuilder.h"
#include "GraphChecker.h"
#include "gtest/gtest.h"
#include "PassBase.h"


namespace ir::tests {
class CompilerTestBase : public ::testing::Test {
public:
    void SetUp() override {
        graph = compiler.CreateNewGraph();
    }

    void TearDown() override {
        ASSERT_TRUE(compiler.DeleteFunctionGraph(graph->GetId()));
        graph = nullptr;
    }

    Graph *GetGraph() {
        return graph;
    }
    InstructionBuilder *GetInstructionBuilder(Graph *targetGraph = nullptr) {
        if (targetGraph == nullptr) {
            targetGraph = GetGraph();
        }
        return targetGraph->GetInstructionBuilder();
    }

    static void compareInstructions(std::vector<InstructionBase *> expected, BasicBlock *bblock) {
        ASSERT_EQ(bblock->GetSize(), expected.size());
        size_t i = 0;
        for (auto *instr : *bblock) {
            ASSERT_EQ(instr, expected[i++]);
        }
    }

    static void VerifyControlAndDataFlowGraphs(Graph *graph) {
        PassManager::Run<GraphChecker>(graph);
    }
    static void VerifyControlAndDataFlowGraphs(const BasicBlock *bblock) {
        GraphChecker::VerifyControlAndDataFlowGraphs(bblock);
    }

public:
    Compiler compiler;

protected:
    Graph *graph = nullptr;
};
}   // namespace ir::tests

#endif  // JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_
