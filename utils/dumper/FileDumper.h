#ifndef JIT_AOT_COMPILERS_COURSE_FILE_DUMPER_H_
#define JIT_AOT_COMPILERS_COURSE_FILE_DUMPER_H_

#include "EventDumper.h"
#include <fstream>


namespace utils::dumper {
class FileDumper : public EventDumper {
public:
    FileDumper(const std::string &name, const std::string &filePath, bool duplicateToStdOut = false)
        : EventDumper(name), filePath(filePath), duplicateToStdOut(duplicateToStdOut),
          fs(filePath, std::fstream::out | std::fstream::app) {}
    ~FileDumper() noexcept override = default;

protected:
    void preDump() override {
        fs << '[' << GetName() << "]: ";
        if (duplicateToStdOut) {
            std::cout << '[' << GetName() << "]: ";
        }
    }
    void dump(std::string &&str) override {
        fs << str;
        if (duplicateToStdOut) {
            std::cout << str;
        }
    }

private:
    std::string filePath;
    bool duplicateToStdOut;

    std::fstream fs;
};
}   // namespace utils::dumper

#endif // JIT_AOT_COMPILERS_COURSE_FILE_DUMPER_H_
