#ifndef JIT_AOT_COMPILERS_COURSE_MACROS_H_
#define JIT_AOT_COMPILERS_COURSE_MACROS_H_

#include "debug.h"


#define UNLIKELY(expr) (__builtin_expect((expr) != 0, false))

#ifdef DEBUG

#define ASSERT_FAIL(expr) utils::AssertionFail((expr), __FILE__, __LINE__, __FUNCTION__)
#define ASSERT(expr)            \
    if (UNLIKELY(!(expr))) {    \
        ASSERT_FAIL(#expr);     \
    }

#define DEBUG_OBJ_DUMP(obj) obj.Dump()

#else

#define ASSERT(expr) static_cast<void>(0)

#define DEBUG_OBJ_DUMP(obj) static_cast<void>(0)

#endif

#define WARNING(mess) utils::PrintWarning((mess), __FILE__, __LINE__, __FUNCTION__)
#define UNREACHABLE(mess) utils::AssertionFail(mess, __FILE__, __LINE__, __FUNCTION__)


#define DEFAULT_DTOR(TypeName) \
    ~TypeName() = default

#define DEFAULT_COPY_CTOR(TypeName) \
    TypeName(const TypeName &) = default;

#define DEFAULT_COPY_OPERATOR(TypeName) \
    TypeName &operator=(const TypeName &) = default

#define DEFAULT_COPY_SEMANTIC(TypeName) \
    DEFAULT_COPY_CTOR(TypeName)         \
    DEFAULT_COPY_OPERATOR(TypeName)

#define DEFAULT_MOVE_CTOR(TypeName) \
    TypeName(TypeName &&) = default;

#define DEFAULT_MOVE_OPERATOR(TypeName) \
    TypeName &operator=(TypeName &&) = default

#define DEFAULT_MOVE_SEMANTIC(TypeName) \
    DEFAULT_MOVE_CTOR(TypeName)         \
    DEFAULT_MOVE_OPERATOR(TypeName)

#define NO_COPY_CTOR(TypeName) \
    TypeName(const TypeName &) = delete;

#define NO_COPY_OPERATOR(TypeName) \
    TypeName &operator=(const TypeName &) = delete

#define NO_COPY_SEMANTIC(TypeName) \
    NO_COPY_CTOR(TypeName)         \
    NO_COPY_OPERATOR(TypeName)

#define NO_MOVE_CTOR(TypeName) \
    TypeName(TypeName &&) = delete;

#define NO_MOVE_OPERATOR(TypeName) \
    TypeName &operator=(TypeName &&) = delete

#define NO_MOVE_SEMANTIC(TypeName) \
    NO_MOVE_CTOR(TypeName)         \
    NO_MOVE_OPERATOR(TypeName)

#endif // JIT_AOT_COMPILERS_COURSE_MACROS_H_
