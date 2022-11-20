#include "ArenaAllocator.h"
#include "sys/mman.h"


namespace utils::memory {
ArenaAllocator::~ArenaAllocator() noexcept {
    ASSERT(arenaList);
    while (arenaList) {
        ASSERT(munmap(arenaList->start, arenaList->GetSize()) == 0);
        auto *next = arenaList->GetNextArena();
        delete arenaList;
        arenaList = next;
    }
}

void *Arena::Alloc(size_t sz, size_t alignSize) {
    ASSERT(IsPowerOfTwo(alignSize));
    void *aligned = std::align(alignSize, sz, end, freeSize);
    if (aligned == nullptr) {
        return nullptr;
    }
    end = VoidPtrT(UintptrT(aligned) + sz);
    freeSize -= sz;
    return aligned;
}

void ArenaAllocator::addNewArena() {
    void *mem = mmap(nullptr, arenaSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(mem != MAP_FAILED);
    auto *newArena = new Arena(mem, arenaSize);
    ASSERT(newArena);
    newArena->SetNextArena(arenaList);
    arenaList = newArena;
}

void *ArenaAllocator::allocate(size_t size) {
    auto addr = arenaList->Alloc(size, alignment);
    if (addr == nullptr) {
        addNewArena();
        return allocate(size);
    }
    return addr;
}

STLCompliantArenaAllocator<int> ArenaAllocator::ToSTL() {
    return STLCompliantArenaAllocator<int>(this);
}
}   // namespace utils::memory
