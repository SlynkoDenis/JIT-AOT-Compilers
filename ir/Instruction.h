#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_

#include <array>
#include "Concepts.h"
#include <cstdint>
#include "macros.h"
#include "Types.h"


namespace ir {
class BasicBlock;

// Opcodes & Conditional Codes
#define INSTS_LIST(DEF) \
    DEF(MUL)            \
    DEF(ADDI)           \
    DEF(MOVI)           \
    DEF(CAST)           \
    DEF(CMP)            \
    DEF(JA)             \
    DEF(JMP)            \
    DEF(RET)

enum class Opcode {
#define OPCODE_DEF(name, ...) name,
    INSTS_LIST(OPCODE_DEF)
#undef OPCODE_DEF
    INVALID,
    NUM_OPCODES = INVALID
};

const char *getOpcodeName(Opcode opcode);

enum class CondCode {
    EQ,
    NE,
    LT,
    GE
};

// Virtual Register
class VReg {
public:
    using Type = uint8_t;

    VReg(uint8_t value) : value(value) {}

    explicit operator uint8_t() {
        return value;
    }

private:
    Type value;
};

// Instructions
class InstructionBase {
public:
    InstructionBase(Opcode opcode, OperandType type)
        : opcode(opcode), prev(nullptr), next(nullptr), parent(nullptr), type(type) {}
    NO_COPY_SEMANTIC(InstructionBase);
    NO_MOVE_SEMANTIC(InstructionBase);
    virtual DEFAULT_DTOR(InstructionBase);

    InstructionBase *GetPrevInstruction() const {
        return prev;
    }
    InstructionBase *GetNextInstruction() const {
        return next;
    }
    BasicBlock *GetBasicBlock() const {
        return parent;
    }
    auto GetOpcode() const {
        return opcode;
    }
    const char *GetOpcodeName() const {
        return getOpcodeName(opcode);
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
    void UnlinkFromParent();
    void InsertBefore(InstructionBase *inst);
    void InsertAfter(InstructionBase *inst);

private:
    Opcode opcode;
    InstructionBase *prev;
    InstructionBase *next;
    BasicBlock *parent;
    OperandType type;
};

class DestVRegMixin {
public:
    DestVRegMixin(VReg vdest) : vdest(vdest) {}

    auto GetDestVReg() const {
        return vdest;
    }
    void SetDestVReg(VReg vreg) {
        vdest = vreg;
    }

private:
    VReg vdest;
};

template <Numeric T>
class ImmediateMixin {
public:
    using Type = T;

    explicit ImmediateMixin(T value) : value(value) {}

    auto GetImm() const {
        return value;
    }
    void SetImm(T new_value) {
        value = new_value;
    }

private:
    T value;
};

class ConditionMixin {
public:
    ConditionMixin(CondCode ccode) : ccode(ccode) {}

    auto GetCondCode() const {
        return ccode;
    }
    void SetCondCode(CondCode cc) {
        ccode = cc;
    }

private:
    CondCode ccode;
};

// Specific instructions
class UnaryRegInstruction : public InstructionBase, public DestVRegMixin {
public:
    UnaryRegInstruction(Opcode opcode, OperandType type, VReg vdest, VReg vreg)
        : InstructionBase(opcode, type), DestVRegMixin(vdest), vreg(vreg) {}

    VReg GetReg() const {
        return vreg;
    }

private:
    VReg vreg;
};

class BinaryRegInstruction : public InstructionBase, public DestVRegMixin {
public:
    BinaryRegInstruction(Opcode opcode, OperandType type, VReg vdest, VReg vreg1, VReg vreg2)
        : InstructionBase(opcode, type), DestVRegMixin(vdest), vreg1(vreg1), vreg2(vreg2) {}

    VReg GetReg1() const {
        return vreg1;
    }
    VReg GetReg2() const {
        return vreg2;
    }

private:
    VReg vreg1;
    VReg vreg2;
};

template <ValidOpType T>
class BinaryImmInstruction : public InstructionBase, public DestVRegMixin, public ImmediateMixin<T> {
public:
    BinaryImmInstruction(Opcode opcode, OperandType type, VReg vdest, VReg vreg, T imm)
        : InstructionBase(opcode, type), DestVRegMixin(vdest), ImmediateMixin<T>(imm), vreg(vreg) {
        ASSERT(type == getOperandType<T>());
    }

    VReg GetReg() const {
        return vreg;
    }

private:
    VReg vreg;
};

class CompareInstruction : public InstructionBase, public ConditionMixin {
public:
    CompareInstruction(Opcode opcode, OperandType type, CondCode ccode, VReg v1, VReg v2)
        : InstructionBase(opcode, type), ConditionMixin(ccode), vreg1(v1), vreg2(v2) {}

private:
    VReg vreg1;
    VReg vreg2;
};

class CastInstruction : public UnaryRegInstruction {
public:
    explicit CastInstruction(OperandType fromType, OperandType toType, VReg vdest, VReg vreg)
        : UnaryRegInstruction(Opcode::CAST, fromType, vdest, vreg), toType(toType) {}

    OperandType GetTargetType() const {
        return toType;
    }

private:
    OperandType toType;
};

template <ValidOpType T>
class JumpInstruction : public InstructionBase, public ImmediateMixin<T> {
public:
    JumpInstruction(Opcode opcode, OperandType type, T imm)
        : InstructionBase(opcode, type), ImmediateMixin<T>(imm) {
        ASSERT(type == getOperandType<T>());
    }

private:
    T dest;
};

class RetInstruction : public InstructionBase {
public:
    RetInstruction(OperandType type, VReg vreg)
        : InstructionBase(Opcode::RET, type), vreg(vreg) {}

private:
    VReg vreg;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
