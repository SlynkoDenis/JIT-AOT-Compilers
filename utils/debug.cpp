#include "debug.h"


void utils::AssertionFail(const char *expr, const char *file, unsigned line, const char *function) {
    std::cerr << "ASSERTION FAILED: " << expr << std::endl;
    std::cerr << "IN " << file << ":" << std::dec << line << ":" << function << std::endl;
    std::terminate();
}

void utils::PrintWarning(const char *mess, const char *file, unsigned line, const char *function) {
    std::cerr << "WARNING: " << mess << std::endl;
    std::cerr << "IN " << file << ":" << std::dec << line << ":" << function << std::endl;
}

void utils::AssertionFail(std::string expr, const char *file, unsigned line, const char *function) {
    AssertionFail(expr.data(), file, line, function);
}

void utils::PrintWarning(std::string mess, const char *file, unsigned line, const char *function) {
    PrintWarning(mess.data(), file, line, function);
}
