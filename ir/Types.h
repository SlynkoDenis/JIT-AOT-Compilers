#ifndef JIT_AOT_COMPILERS_COURSE_TYPES_H_
#define JIT_AOT_COMPILERS_COURSE_TYPES_H_

#include <array>
#include <cstdint>
#include <limits>
#include "macros.h"
#include <type_traits>


namespace ir {
enum class OperandType {
    VOID,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
    INVALID,
    NUM_TYPES = INVALID
};

constexpr std::array<uint64_t, static_cast<size_t>(OperandType::NUM_TYPES)> maxValues{
    0,
    std::numeric_limits<int8_t>::max(),
    std::numeric_limits<int16_t>::max(),
    std::numeric_limits<int32_t>::max(),
    std::numeric_limits<int64_t>::max(),
    std::numeric_limits<uint8_t>::max(),
    std::numeric_limits<uint16_t>::max(),
    std::numeric_limits<uint32_t>::max(),
    std::numeric_limits<uint64_t>::max(),
};

template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept ValidOpType = IsAnyOf<T, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>;

constexpr inline size_t GetTypeBitSize(OperandType type) {
    switch (type) {
    case OperandType::I8:
    case OperandType::U8:
        return 8;
    case OperandType::I16:
    case OperandType::U16:
        return 16;
    case OperandType::I32:
    case OperandType::U32:
        return 32;
    case OperandType::I64:
    case OperandType::U64:
        return 64;
    default:
        UNREACHABLE("");
        return 0;
    }
}

constexpr inline int64_t ToSigned(uint64_t value, OperandType type) {
    switch (type) {
    case OperandType::U8:
        return static_cast<int64_t>(static_cast<int8_t>(value));
    case OperandType::U16:
        return static_cast<int64_t>(static_cast<int16_t>(value));
    case OperandType::U32:
        return static_cast<int64_t>(static_cast<int32_t>(value));
    case OperandType::U64:
        return static_cast<int64_t>(static_cast<int64_t>(value));
    default:
        return static_cast<int64_t>(value);
    }
}

constexpr inline uint64_t GetMaxValue(OperandType type) {
    ASSERT(type != OperandType::INVALID);
    return maxValues[static_cast<size_t>(type)];
}
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_TYPES_H_
