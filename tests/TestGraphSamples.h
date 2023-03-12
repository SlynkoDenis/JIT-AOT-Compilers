#ifndef JIT_AOT_COMPILERS_COURSE_TEST_GRAPH_SAMPLES_H_
#define JIT_AOT_COMPILERS_COURSE_TEST_GRAPH_SAMPLES_H_

#include "CompilerTestBase.h"


namespace ir::tests {
class TestGraphSamples : public CompilerTestBase {
public:
    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase0();
    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase1();
    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase2();
    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase3();
};
}   // namespace ir::tests

#endif  // JIT_AOT_COMPILERS_COURSE_TEST_GRAPH_SAMPLES_H_
