#ifndef JIT_AOT_COMPILERS_COURSE_COMPILER_H_
#define JIT_AOT_COMPILERS_COURSE_COMPILER_H_

#include "arena/ArenaAllocator.h"
#include "Graph.h"
#include "InstructionBuilder.h"
#include "IRBuilder.h"
#include "macros.h"


class CompilerBase {
public:
    CompilerBase() = default;
    NO_COPY_SEMANTIC(CompilerBase);
    NO_MOVE_SEMANTIC(CompilerBase);
    virtual DEFAULT_DTOR(CompilerBase);
};

class Compiler : public CompilerBase {
public:
    Compiler() : allocator(), instrBuilder(&allocator), irBuilder(&allocator) {}

    ir::InstructionBuilder &GetInstructionBuilder() {
        return instrBuilder;
    }
    ir::IRBuilder &GetIRBuilder() {
        return irBuilder;
    }

private:
    utils::memory::ArenaAllocator allocator;

    ir::InstructionBuilder instrBuilder;
    ir::IRBuilder irBuilder;
};

#endif
