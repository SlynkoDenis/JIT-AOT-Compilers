#ifndef JIT_AOT_COMPILERS_COURSE_ARENA_ALLOCATOR_H_
#define JIT_AOT_COMPILERS_COURSE_ARENA_ALLOCATOR_H_

#include "macros.h"
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>


namespace utils::memory {
constexpr size_t DEFAULT_ALIGNMENT = alignof(uintptr_t);

inline constexpr size_t AlignUp(size_t addr, size_t page_size) {
    return ((addr + (page_size - 1)) / page_size) * page_size;
}

inline constexpr bool IsPowerOfTwo(size_t val) {
    return (val & (val - 1)) == 0;
}

inline uintptr_t UintptrT(void *addr) {
    return reinterpret_cast<uintptr_t>(addr);
}

inline void *VoidPtrT(uintptr_t addr) {
    return reinterpret_cast<void *>(addr);
}


class Arena {
public:
    Arena(void *mem, size_t size)
        : size(size), freeSize(size), start(mem), end(mem) {
        ASSERT(mem);
    }
    DEFAULT_COPY_SEMANTIC(Arena);
    DEFAULT_MOVE_SEMANTIC(Arena);
    virtual DEFAULT_DTOR(Arena);

    size_t GetSize() const {
        return size;
    }
    size_t GetFreeSize() const {
        return freeSize;
    }
    Arena *GetNextArena() {
        return nextArena;
    }
    const Arena *GetNextArena() const {
        return nextArena;
    }

    void SetNextArena(Arena *arena) {
        nextArena = arena;
    }
    [[nodiscard]] void *Alloc(size_t sz, size_t alignSize);

private:
    size_t size;
    size_t freeSize;

    void *start;
    void *end;

    Arena *nextArena = nullptr;

    friend class ArenaAllocator;
};


template <typename T>
concept NonVoid = (!std::is_same_v<T, void>);

template <NonVoid T>
class STLCompliantArenaAllocator;

// TODO: make possible two implementations: arena and default allocator based
template <NonVoid T>
using ArenaVector = std::vector<T, STLCompliantArenaAllocator<T>>;

template <NonVoid T>
using ArenaSet = std::set<T, std::less<T>, STLCompliantArenaAllocator<T>>;

template <NonVoid KeyT, NonVoid ValueT>
using ArenaUnorderedMap = std::unordered_map<
    KeyT,
    ValueT,
    std::hash<KeyT>,
    std::equal_to<KeyT>,
    STLCompliantArenaAllocator<std::pair<const KeyT, ValueT>>>;

// TODO: consider using jemalloc
class ArenaAllocator final {
public:
    explicit ArenaAllocator(size_t arenaSize = DEFAULT_ARENA_SIZE,
                            size_t alignment = DEFAULT_ALIGNMENT)
        : arenaList(nullptr),
          arenaSize(AlignUp(arenaSize, PAGE_SIZE)),
          alignment(alignment)
    {
        addNewArena();
    }
    NO_COPY_SEMANTIC(ArenaAllocator);
    DEFAULT_MOVE_SEMANTIC(ArenaAllocator);
    ~ArenaAllocator() noexcept;

    size_t GetFreeSize() const {
        return arenaList->GetFreeSize();
    }

    STLCompliantArenaAllocator<int> ToSTL();

    template <typename T>
    [[nodiscard]] T *AllocateArray(size_t n) {
        auto p = allocate(sizeof(T) * n);
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        return static_cast<T*>(p);
    }

    template <NonVoid T>
    [[nodiscard]] void *NewRaw() {
        return allocate(sizeof(T));
    }
    template <NonVoid T, typename... ArgsT>
    [[nodiscard]] T *New(ArgsT &&... args) {
        auto p = NewRaw<T>();
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        new(p) T(std::forward<ArgsT>(args)...);
        return reinterpret_cast<T*>(p);
    }

    template <NonVoid T, typename... ArgsT>
    [[nodiscard]] inline ArenaVector<T> *NewVector(ArgsT &&... args);

    template <NonVoid KeyT, NonVoid ValueT, typename... ArgsT>
    [[nodiscard]] inline ArenaUnorderedMap<KeyT, ValueT> *NewUnorderedMap(ArgsT &&... args);

public:
    static constexpr size_t DEFAULT_ARENA_SIZE = 4096;

private:
    void addNewArena();
    void *allocate(size_t size);

private:
    Arena *arenaList;
    size_t arenaSize;

    size_t alignment;

    static constexpr size_t PAGE_SIZE = 4096;
};

template <NonVoid T>
class STLCompliantArenaAllocator {
public:
    using pointer = T*;
    using value_type = T;
    using size_type = size_t;
    using difference_type = size_t;

    explicit STLCompliantArenaAllocator(ArenaAllocator *const alloc) : allocator(alloc) {
        ASSERT(allocator);
    }
    template <NonVoid V>
    STLCompliantArenaAllocator(const STLCompliantArenaAllocator<V> &other)
        : allocator(other.GetAllocator()) {}

    DEFAULT_COPY_SEMANTIC(STLCompliantArenaAllocator);
    DEFAULT_MOVE_SEMANTIC(STLCompliantArenaAllocator);
    virtual DEFAULT_DTOR(STLCompliantArenaAllocator);

    [[nodiscard]] pointer allocate(size_type n) {
        return allocator->template AllocateArray<T>(n);
    }

    void deallocate([[maybe_unused]] pointer p, [[maybe_unused]] size_type n) {}

    ArenaAllocator *GetAllocator() const {
        return allocator;
    }

private:
    ArenaAllocator *const allocator;
};

template <NonVoid T>
inline bool operator==(const STLCompliantArenaAllocator<T> &lhs,
                       const STLCompliantArenaAllocator<T> &rhs) {
    return lhs.GetAllocator() == rhs.GetAllocator();
}


template <NonVoid T, typename... ArgsT>
[[nodiscard]] inline ArenaVector<T> *ArenaAllocator::NewVector(ArgsT &&... args) {
    return New<ArenaVector<T>>(std::forward<ArgsT>(args)..., ToSTL());
}

template <NonVoid KeyT, NonVoid ValueT, typename... ArgsT>
[[nodiscard]] inline ArenaUnorderedMap<KeyT, ValueT> *ArenaAllocator::NewUnorderedMap(ArgsT &&... args)
{
    return New<ArenaUnorderedMap<KeyT, ValueT>>(std::forward<ArgsT>(args)..., ToSTL());
}
}   // namespace utils::memory

#endif // JIT_AOT_COMPILERS_COURSE_ARENA_ALLOCATOR_H_
