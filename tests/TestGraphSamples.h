#ifndef JIT_AOT_COMPILERS_COURSE_TEST_GRAPH_SAMPLES_H_
#define JIT_AOT_COMPILERS_COURSE_TEST_GRAPH_SAMPLES_H_

#include "Graph.h"
#include "IRBuilder.h"


namespace ir::tests {
class TestGraphSamples {
public:
    virtual ~TestGraphSamples() noexcept = default;

    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase1();
    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase2();
    std::pair<Graph *, std::vector<BasicBlock *>> BuildCase3();

public:
    IRBuilder irBuilder;
};
}   // namespace ir::tests

#endif  // JIT_AOT_COMPILERS_COURSE_TEST_GRAPH_SAMPLES_H_
