#ifndef JIT_AOT_COMPILERS_COURSE_STD_OUTPUT_DUMPER_H_
#define JIT_AOT_COMPILERS_COURSE_STD_OUTPUT_DUMPER_H_

#include "EventDumper.h"


namespace utils::dumper {
class StdOutputDumper : public EventDumper {
public:
    StdOutputDumper(const std::string &name) : EventDumper(name) {}
    ~StdOutputDumper() noexcept override = default;

    static EventDumper *GetDefaultDumper(memory::ArenaAllocator *const allocator) {
        return EventDumper::AddDumper<StdOutputDumper>(allocator, "StdOutputDumper").second;
    }

protected:
    void preDump() override {
    }
    void dump(std::string &&str) override {
        std::cout << str;
    }
};
}   // namespace utils::dumper

#endif  // JIT_AOT_COMPILERS_COURSE_STD_OUTPUT_DUMPER_H_
