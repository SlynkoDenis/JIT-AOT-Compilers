#ifndef JIT_AOT_COMPILERS_COURSE_HELPERS_H_
#define JIT_AOT_COMPILERS_COURSE_HELPERS_H_

#include <type_traits>


namespace utils {
template <typename E>
constexpr inline auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

template <typename E>
constexpr inline auto underlying_logic_or(E e) noexcept {
    return to_underlying(e);
}

template <typename E, typename... Eout>
constexpr inline auto underlying_logic_or(E e, Eout... out) noexcept
requires (std::is_same_v<std::remove_cv_t<E>, std::remove_cv_t<Eout>> && ...) {
    return underlying_logic_or(e) | underlying_logic_or(out...);
}

struct expand_t {
template <typename... T> expand_t(T...) {}
};
}   // namespace utils

#endif // JIT_AOT_COMPILERS_COURSE_HELPERS_H_
