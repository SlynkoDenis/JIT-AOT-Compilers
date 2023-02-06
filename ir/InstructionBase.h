#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BASE_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BASE_H_

#include "Concepts.h"
#include <cstdint>
#include "helpers.h"
#include "macros.h"
#include "marker/marker.h"
#include "Types.h"
#include "Users.h"


namespace ir {
class BasicBlock;

using utils::memory::ArenaAllocator;

// Opcodes & Conditional Codes
// instruction which shouldn't be collected by DCE are placed in start of the list
#define INSTS_LIST(DEF) \
    DEF(DIV)            \
    DEF(DIVI)           \
    DEF(CALL)           \
    DEF(LOAD)           \
    DEF(STORE)          \
    DEF(CMP)            \
    DEF(JCMP)           \
    DEF(JMP)            \
    DEF(RET)            \
    DEF(CONST)          \
    DEF(NOT)            \
    DEF(AND)            \
    DEF(OR)             \
    DEF(XOR)            \
    DEF(NEG)            \
    DEF(ADD)            \
    DEF(SUB)            \
    DEF(MUL)            \
    DEF(SRA)            \
    DEF(SLA)            \
    DEF(SLL)            \
    DEF(ANDI)           \
    DEF(ORI)            \
    DEF(XORI)           \
    DEF(ADDI)           \
    DEF(SUBI)           \
    DEF(MULI)           \
    DEF(SRAI)           \
    DEF(SLAI)           \
    DEF(SLLI)           \
    DEF(CAST)           \
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

// Instructions properties, used in optimizations
using InstructionPropT = uint8_t;

enum class InstrProp : InstructionPropT {
    ARITH = 0b1,
    MEM = 0b10,
    COMMUTABLE = 0b100,
    JUMP = 0b1000,
    INPUT = 0b10000,
};

// Instructions
class InstructionBase : public Markable, public Users {
public:
    InstructionBase(Opcode opcode,
                    OperandType type,
                    ArenaAllocator *const allocator,
                    size_t id = INVALID_ID,
                    InstructionPropT prop = 0)
        : Users(allocator),
          id(id),
          opcode(opcode),
          type(type),
          properties(prop) {}
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
    InstructionPropT GetProperties() const {
        return properties;
    }
    bool SatisfiesProperty(InstrProp prop) const {
        return GetProperties() & utils::to_underlying(prop);
    }

    bool IsInputArgument() const {
        return opcode == Opcode::ARG;
    }
    bool IsPhi() const {
        return opcode == Opcode::PHI;
    }
    bool IsConst() const {
        return opcode == Opcode::CONST;
    }
    bool HasInputs() const {
        return SatisfiesProperty(InstrProp::INPUT);
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
    void SetProperty(InstrProp prop) {
        properties |= utils::to_underlying(prop);
    }
    void SetProperty(InstructionPropT prop) {
        properties |= prop;
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

    InstructionBase *prev = nullptr;
    InstructionBase *next = nullptr;

    BasicBlock *parent = nullptr;

    InstructionPropT properties = 0;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BASE_H_
