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
    DEF(RET)            \
    DEF(PHI)

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

    auto GetValue() const {
        return value;
    }

private:
    Type value;
};

inline bool operator==(const VReg& lhs, const VReg& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

// Instructions
class InstructionBase {
public:
    InstructionBase(Opcode opcode, OperandType type)
        : opcode(opcode), prev(nullptr), next(nullptr), parent(nullptr), type(type) {}
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

    auto GetVReg() const {
        return vreg;
    }
    void SetVReg(VReg newVReg) {
        vreg = newVReg;
    }

private:
    VReg vreg;
};

class BinaryRegInstruction : public InstructionBase, public DestVRegMixin {
public:
    BinaryRegInstruction(Opcode opcode, OperandType type, VReg vdest, VReg vreg1, VReg vreg2)
        : InstructionBase(opcode, type), DestVRegMixin(vdest), vreg1(vreg1), vreg2(vreg2) {}

    auto GetVReg1() const {
        return vreg1;
    }
    auto GetVReg2() const {
        return vreg2;
    }
    void SetVReg1(VReg newVReg) {
        vreg1 = newVReg;
    }
    void SetVReg2(VReg newVReg) {
        vreg2 = newVReg;
    }

private:
    VReg vreg1;
    VReg vreg2;
};

class BinaryImmInstruction : public InstructionBase, public DestVRegMixin, public ImmediateMixin<uint64_t> {
public:
    BinaryImmInstruction(Opcode opcode, OperandType type, VReg vdest, VReg vreg, uint64_t imm)
        : InstructionBase(opcode, type), DestVRegMixin(vdest), ImmediateMixin<uint64_t>(imm), vreg(vreg) {
    }

    auto GetVReg() const {
        return vreg;
    }
    void SetVReg(VReg newVReg) {
        vreg = newVReg;
    }

private:
    VReg vreg;
};

class MoveImmediateInstruction : public InstructionBase, public DestVRegMixin, public ImmediateMixin<uint64_t> {
public:
    MoveImmediateInstruction(Opcode opcode, OperandType type, VReg vdest, uint64_t imm)
        : InstructionBase(opcode, type), DestVRegMixin(vdest), ImmediateMixin<uint64_t>(imm) {
    }
};

class CompareInstruction : public InstructionBase, public ConditionMixin {
public:
    CompareInstruction(Opcode opcode, OperandType type, CondCode ccode, VReg v1, VReg v2)
        : InstructionBase(opcode, type), ConditionMixin(ccode), vreg1(v1), vreg2(v2) {}

    auto GetVReg1() const {
        return vreg1;
    }
    auto GetVReg2() const {
        return vreg2;
    }
    void SetVReg1(VReg newVReg) {
        vreg1 = newVReg;
    }
    void SetVReg2(VReg newVReg) {
        vreg2 = newVReg;
    }

private:
    VReg vreg1;
    VReg vreg2;
};

class CastInstruction : public UnaryRegInstruction {
public:
    explicit CastInstruction(OperandType fromType, OperandType toType, VReg vdest, VReg vreg)
        : UnaryRegInstruction(Opcode::CAST, fromType, vdest, vreg), toType(toType) {}

    auto GetTargetType() const {
        return toType;
    }
    void SetTargetType(OperandType newType) {
        toType = newType;
    }

private:
    OperandType toType;
};

class JumpInstruction : public InstructionBase, public ImmediateMixin<uint64_t> {
public:
    JumpInstruction(Opcode opcode, uint64_t imm)
        : InstructionBase(opcode, OperandType::I64), ImmediateMixin<uint64_t>(imm) {}
};

class RetInstruction : public InstructionBase {
public:
    RetInstruction(OperandType type, VReg vreg)
        : InstructionBase(Opcode::RET, type), vreg(vreg) {}

    auto GetVReg() const {
        return vreg;
    }
    void SetVReg(VReg newVReg) {
        vreg = newVReg;
    }

private:
    VReg vreg;
};

class PhiInstruction : public BinaryRegInstruction {
public:
    PhiInstruction(OperandType type, VReg vdest, VReg vreg1, VReg vreg2)
        : BinaryRegInstruction(Opcode::PHI, type, vdest, vreg1, vreg2) {}
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
