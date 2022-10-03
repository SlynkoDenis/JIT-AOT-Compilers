#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_

#include "Concepts.h"
#include "Graph.h"
#include "macros.h"
#include <vector>


namespace ir {
class InstructionBuilder {
public:
    InstructionBuilder() = default;
    NO_COPY_SEMANTIC(InstructionBuilder);
    NO_MOVE_SEMANTIC(InstructionBuilder);
    virtual ~InstructionBuilder() noexcept {
        Clear();
    }

    void PushBackInstruction(BasicBlock *bblock, InstructionBase *instr) {
        bblock->PushBackInstruction(instr);
    }
    template <InstructionType... T>
    void PushBackInstruction(BasicBlock *bblock, InstructionBase *instr, T *... reminder) {
        bblock->PushBackInstruction(instr);
        PushBackInstruction(bblock, reminder...);
    }

    void PushForwardInstruction(BasicBlock *bblock, InstructionBase *instr) {
        bblock->PushForwardInstruction(instr);
    }
    template <InstructionType... T>
    void PushForwardInstruction(BasicBlock *bblock, InstructionBase *instr, T *... reminder) {
        bblock->PushForwardInstruction(instr);
        PushForwardInstruction(bblock, reminder...);
    }

#define CREATE_INST(name, ...)              \
    auto *inst = new name(__VA_ARGS__);     \
    instrs.push_back(inst);                 \
    return inst

    BinaryRegInstruction *CreateMul(OperandType type, VReg vdest, VReg v1, VReg v2) {
        CREATE_INST(BinaryRegInstruction, Opcode::MUL, type, vdest, v1, v2);
    }
    template <ValidOpType T>
    BinaryImmInstruction<T> *CreateAddi(OperandType type, VReg vdest, VReg vreg, T imm) {
        CREATE_INST(BinaryImmInstruction, Opcode::ADDI, type, vdest, vreg, imm);
    }
    template <ValidOpType T>
    BinaryImmInstruction<T> *CreateMovi(OperandType type, VReg vdest, VReg vreg, T imm) {
        CREATE_INST(BinaryImmInstruction, Opcode::MOVI, type, vdest, vreg, imm);
    }
    CastInstruction *CreateCast(OperandType fromType, OperandType toType, VReg vdest, VReg vreg) {
        CREATE_INST(CastInstruction, fromType, toType, vdest, vreg);
    }
    CompareInstruction *CreateCmp(OperandType type, CondCode ccode, VReg v1, VReg v2) {
        CREATE_INST(CompareInstruction, Opcode::CMP, type, ccode, v1, v2);
    }
    template <ValidOpType T>
    JumpInstruction<T> *CreateJa(OperandType type, T imm) {
        CREATE_INST(JumpInstruction, Opcode::JA, type, imm);
    }
    template <ValidOpType T>
    JumpInstruction<T> *CreateJmp(OperandType type, T imm) {
        CREATE_INST(JumpInstruction, Opcode::JMP, type, imm);
    }
    RetInstruction *CreateRet(OperandType type, VReg vreg) {
        CREATE_INST(RetInstruction, type, vreg);
    }

#undef CREATE_INST

    void Clear() noexcept {
        for (auto &instr : instrs) {
            delete instr;
        }
    }

private:
    std::vector<InstructionBase *> instrs;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_
