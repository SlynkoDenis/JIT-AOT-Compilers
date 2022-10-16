#ifndef JIT_AOT_COMPILERS_COURSE_CONCEPTS_H_
#define JIT_AOT_COMPILERS_COURSE_CONCEPTS_H_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include "Types.h"


namespace ir {
template <typename T>
concept Numeric = std::is_arithmetic<T>::value;

template <typename BaseType, typename... SubType>
concept InstructionType = (std::is_base_of<BaseType, SubType>::value && ...);

template <typename TargetType, typename... InputsTypes>
concept IsSameType = (std::is_same<TargetType, InputsTypes>::value && ...);
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_CONCEPTS_H_
