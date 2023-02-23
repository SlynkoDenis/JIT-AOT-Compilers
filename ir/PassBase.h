#ifndef JIT_AOT_COMPILERS_COURSE_PASS_BASE_H_
#define JIT_AOT_COMPILERS_COURSE_PASS_BASE_H_

#include "dumper/DummyDumper.h"
#include "dumper/EventDumper.h"
#include "Graph.h"


namespace ir {
class PassBase;

class PassManager final {
public:
    PassManager() = delete;
    NO_COPY_SEMANTIC(PassManager);
    NO_MOVE_SEMANTIC(PassManager);
    DEFAULT_DTOR(PassManager);

    template <typename PassT, typename... ArgsT>
    static void Run(Graph *graph, ArgsT... args)
    requires std::is_base_of_v<PassBase, PassT>
    {
        if constexpr (utils::has_set_flag_v<PassT>) {
            static_assert(std::is_same_v<std::remove_cv_t<decltype(PassT::SET_FLAG)>, AnalysisFlag>);
            if (graph->IsAnalysisValid(PassT::SET_FLAG)) {
                return;
            }
            PassT(graph, args...).Run();
            graph->SetAnalysisValid(PassT::SET_FLAG, true);
        } else {
            PassT(graph, args...).Run();
        }
    }

    template <AnalysisFlag... Flags>
    static void SetInvalid(Graph *graph) {
        utils::expand_t{(graph->SetAnalysisValid(Flags, false), void(), 0)...};
    }
};

class PassBase {
public:
    PassBase(Graph *graph, bool shouldDump) : graph(graph)
    {
        ASSERT(graph);
        if (shouldDump) {
            dumper = utils::dumper::EventDumper::AddDumper(
                graph->GetAllocator(), GetName()).second;
        } else {
            dumper = utils::dumper::EventDumper::AddDumper<utils::dumper::DummyDumper>(
                graph->GetAllocator(), utils::dumper::DummyDumper::DUMPER_NAME).second;
        }
    }
    NO_COPY_SEMANTIC(PassBase);
    NO_MOVE_SEMANTIC(PassBase);
    virtual ~PassBase() noexcept = default;

    virtual void Run() = 0;

    virtual const char *GetName() const {
        return "base";
    }

protected:
    Graph *graph;

    utils::dumper::EventDumper *dumper;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_PASS_BASE_H_
