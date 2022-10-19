#ifndef JIT_AOT_COMPILERS_COURSE_DSU2_H_
#define JIT_AOT_COMPILERS_COURSE_DSU2_H_

#include "BasicBlock.h"
#include "macros.h"
#include <numeric>
#include <stdexcept>
#include <vector>


namespace ir {
class DSU final {
public:
    DSU() : labels(nullptr), sdoms(nullptr) {}
    DSU(std::vector<BasicBlock *> &labels, const std::vector<size_t> &sdoms)
        : universum(labels.size(), nullptr), labels(&labels), sdoms(&sdoms) {}

    BasicBlock *Find(BasicBlock *bblock);

    size_t GetSize() const {
        return universum.size();
    }

    void Unite(BasicBlock *target, BasicBlock *parent) {
        universum[target->GetId()] = parent;
    }

    void Dump() const {
        std::cout << "===== DSU =====\n";
        for (size_t i = 0; i < GetSize(); ++i) {
            if (universum[i]) {
                std::cout << "= BBlock #" << i << ": " << universum[i]->GetId() << std::endl;
            } else {
                std::cout << "= BBlock #" << i << " is root\n";
            }
        }
        std::cout << "===============" << std::endl;
    }

private:
    void compressUniversum(BasicBlock *bblock);

private:
    std::vector<BasicBlock *> universum;
    std::vector<BasicBlock *> *labels;
    const std::vector<size_t> *sdoms;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DSU2_H_
