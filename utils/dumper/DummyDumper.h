#ifndef JIT_AOT_COMPILERS_COURSE_DUMMY_DUMPER_H_
#define JIT_AOT_COMPILERS_COURSE_DUMMY_DUMPER_H_

#include "EventDumper.h"


namespace utils::dumper {
class DummyDumper : public EventDumper {
public:
    DummyDumper(const std::string &name)
        : EventDumper(name) {}
    ~DummyDumper() noexcept override = default;

protected:
    void preDump() override {}
    void dump([[maybe_unused]] std::string &&str) override {}
};
}   // namespace utils::dumper

#endif // JIT_AOT_COMPILERS_COURSE_DUMMY_DUMPER_H_
