#ifndef JIT_AOT_COMPILERS_COURSE_TYPES_H_
#define JIT_AOT_COMPILERS_COURSE_TYPES_H_

#include <cstdint>
#include <type_traits>
#include "macros.h"


namespace ir {
enum class OperandType {
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

template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept ValidOpType = IsAnyOf<T, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, uint64_t>;

template <ValidOpType T>
constexpr OperandType getOperandType() {
    if constexpr (std::is_same_v<T, int8_t>) {
        return OperandType::I8;
    }
    if constexpr (std::is_same_v<T, int16_t>) {
        return OperandType::I16;
    }
    if constexpr (std::is_same_v<T, int32_t>) {
        return OperandType::I32;
    }
    if constexpr (std::is_same_v<T, int64_t>) {
        return OperandType::I64;
    }
    if constexpr (std::is_same_v<T, uint8_t>) {
        return OperandType::U8;
    }
    if constexpr (std::is_same_v<T, uint16_t>) {
        return OperandType::U16;
    }
    if constexpr (std::is_same_v<T, uint32_t>) {
        return OperandType::U32;
    }
    if constexpr (std::is_same_v<T, uint64_t>) {
        return OperandType::U64;
    }
    UNREACHABLE("");
}
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_TYPES_H_
