#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BASE_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BASE_H_

#include "Concepts.h"
#include <cstdint>
#include "macros.h"
#include "Types.h"


namespace ir {
class BasicBlock;

// Opcodes & Conditional Codes
#define INSTS_LIST(DEF) \
    DEF(CONST)          \
    DEF(MUL)            \
    DEF(ADDI)           \
    DEF(CAST)           \
    DEF(CMP)            \
    DEF(JA)             \
    DEF(JMP)            \
    DEF(RET)            \
    DEF(PHI)            \
    DEF(ARG)

enum class Opcode {
#define OPCODE_DEF(name, ...) name,
    INSTS_LIST(OPCODE_DEF)
#undef OPCODE_DEF
    INVALID,
    NUM_OPCODES = INVALID
};

const char *getOpcodeName(Opcode opcode);

// Instructions
class InstructionBase {
public:
    InstructionBase(Opcode opcode, OperandType type, size_t id = INVALID_ID)
        : id(id),
          opcode(opcode),
          type(type),
          prev(nullptr),
          next(nullptr),
          parent(nullptr) {}
    NO_COPY_SEMANTIC(InstructionBase);
    NO_MOVE_SEMANTIC(InstructionBase);
    virtual DEFAULT_DTOR(InstructionBase);

    InstructionBase *GetPrevInstruction() {
        return prev;
    }
    const InstructionBase *GetPrevInstruction() const {
        return prev;
    }
    InstructionBase *GetNextInstruction() {
        return next;
    }
    const InstructionBase *GetNextInstruction() const {
        return next;
    }
    BasicBlock *GetBasicBlock() {
        return parent;
    }
    const BasicBlock *GetBasicBlock() const {
        return parent;
    }
    auto GetOpcode() const {
        return opcode;
    }
    auto GetType() const {
        return type;
    }
    const char *GetOpcodeName() const {
        return getOpcodeName(opcode);
    }
    size_t GetId() const {
        return id;
    }

    bool IsInputArgument() const {
        return opcode == Opcode::ARG;
    }
    bool IsPhi() const {
        return opcode == Opcode::PHI;
    }

    void SetPrevInstruction(InstructionBase *inst) {
        prev = inst;
    }
    void SetNextInstruction(InstructionBase *inst) {
        next = inst;
    }
    void SetBasicBlock(BasicBlock *bblock) {
        parent = bblock;
    }
    void SetType(OperandType newType) {
        type = newType;
    }
    void SetId(size_t newId) {
        id = newId;
    }
    void UnlinkFromParent();
    void InsertBefore(InstructionBase *inst);
    void InsertAfter(InstructionBase *inst);

    NO_NEW_DELETE;

public:
    static constexpr size_t INVALID_ID = static_cast<size_t>(0) - 1;

private:
    size_t id;

    Opcode opcode;
    OperandType type;

    InstructionBase *prev;
    InstructionBase *next;

    BasicBlock *parent;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BASE_H_
