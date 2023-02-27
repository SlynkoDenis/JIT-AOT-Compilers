#include "debug.h"
#include "logger.h"


namespace utils {
void AssertionFail(const char *expr, const char *file, unsigned line, const char *function) {
    Logger::GetRoot() << log4cpp::Priority::CRIT << "ASSERTION FAILED: " << expr;
    Logger::GetRoot() << log4cpp::Priority::CRIT << "IN " << file << ":" << std::dec << line << ":" << function;
    std::terminate();
}

void PrintWarning(const char *mess, const char *file, unsigned line, const char *function) {
    Logger::GetRoot() << log4cpp::Priority::WARN << "WARNING: " << mess;
    Logger::GetRoot() << log4cpp::Priority::WARN << "IN " << file << ":" << std::dec << line << ":" << function;
}

void AssertionFail(std::string expr, const char *file, unsigned line, const char *function) {
    AssertionFail(expr.data(), file, line, function);
}

void PrintWarning(std::string mess, const char *file, unsigned line, const char *function) {
    PrintWarning(mess.data(), file, line, function);
}
}   // namespace utils
