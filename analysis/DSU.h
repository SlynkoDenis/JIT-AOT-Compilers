#ifndef JIT_AOT_COMPILERS_COURSE_DSU_H_
#define JIT_AOT_COMPILERS_COURSE_DSU_H_

#include "arena/ArenaAllocator.h"
#include "BasicBlock.h"
#include "macros.h"
#include <numeric>
#include <stdexcept>
#include <vector>


namespace ir {
using utils::memory::ArenaVector;

class DSU final {
public:
    DSU() = delete;
    DSU(utils::memory::ArenaVector<BasicBlock *> *labels,
        const utils::memory::ArenaVector<size_t> *sdoms,
        ArenaAllocator *const allocator)
        : universum(labels->size(), nullptr, allocator->ToSTL()),
          labels(labels),
          sdoms(sdoms) {}
    DEFAULT_COPY_SEMANTIC(DSU);
    DEFAULT_MOVE_SEMANTIC(DSU);
    DEFAULT_DTOR(DSU);

    BasicBlock *Find(BasicBlock *bblock);

    size_t GetSize() const {
        return universum.size();
    }

    void Unite(BasicBlock *target, BasicBlock *parent) {
        setUniversum(target->GetId(), parent);
    }

    void Dump() const;

private:
    BasicBlock *getUniversum(size_t id) {
        return universum.at(id);
    }
    const BasicBlock *getUniversum(size_t id) const {
        return universum.at(id);
    }
    void setUniversum(size_t id, BasicBlock *bblock) {
        universum.at(id) = bblock;
    }

    void compressUniversum(BasicBlock *bblock);

private:
    ArenaVector<BasicBlock *> universum;
    ArenaVector<BasicBlock *> *labels;
    const ArenaVector<size_t> *sdoms;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_DSU_H_
