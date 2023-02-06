#ifndef JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_
#define JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_

#include "Compiler.h"
#include "gtest/gtest.h"


namespace ir::tests {
class CompilerTestBase : public ::testing::Test {
public:
    void SetUp() override {
        GetIRBuilder().CreateGraph();
    }

    void TearDown() override {
        GetIRBuilder().Clear();
    }

    IRBuilder &GetIRBuilder() {
        return compiler.GetIRBuilder();
    }
    InstructionBuilder &GetInstructionBuilder() {
        return compiler.GetInstructionBuilder();
    }

    static void compareInstructions(std::vector<InstructionBase *> expected, BasicBlock *bblock) {
        ASSERT_EQ(bblock->GetSize(), expected.size());
        size_t i = 0;
        for (auto *instr : *bblock) {
            ASSERT_EQ(instr, expected[i++]);
        }
    }

public:
    Compiler compiler;
};
}   // namespace ir::tests

#endif  // JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_
