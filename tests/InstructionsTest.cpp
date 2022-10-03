#include "gtest/gtest.h"
#include "InstructionBuilder.h"
#include "IRBuilder.h"


namespace ir::tests {
class InstructionsTest : public ::testing::Test {
public:
    InstructionsTest() = default;

    virtual void TearDown() {
        instrBuilder.Clear();
        irBuilder.Clear();
    }

    InstructionBuilder instrBuilder;
    IRBuilder irBuilder;
};

TEST_F(InstructionsTest, TestMov) {
    // TBD
}
}   // namespace ir::tests
