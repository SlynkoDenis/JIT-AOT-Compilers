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

public:
    Compiler compiler;
};
}   // namespace ir::tests

#endif  // JIT_AOT_COMPILERS_COURSE_COMPILER_TEST_BASE_H_
