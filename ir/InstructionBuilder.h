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
    BinaryImmInstruction *CreateAddi(OperandType type, VReg vdest, VReg vreg, T imm) {
        CREATE_INST(BinaryImmInstruction, Opcode::ADDI, type, vdest, vreg, imm);
    }
    template <ValidOpType T>
    MoveImmediateInstruction *CreateMovi(OperandType type, VReg vdest, T imm) {
        CREATE_INST(MoveImmediateInstruction, Opcode::MOVI, type, vdest, imm);
    }
    CastInstruction *CreateCast(OperandType fromType, OperandType toType, VReg vdest, VReg vreg) {
        CREATE_INST(CastInstruction, fromType, toType, vdest, vreg);
    }
    CompareInstruction *CreateCmp(OperandType type, CondCode ccode, VReg v1, VReg v2) {
        CREATE_INST(CompareInstruction, Opcode::CMP, type, ccode, v1, v2);
    }
    JumpInstruction *CreateJa(int64_t imm) {
        CREATE_INST(JumpInstruction, Opcode::JA, imm);
    }
    JumpInstruction *CreateJmp(int64_t imm) {
        CREATE_INST(JumpInstruction, Opcode::JMP, imm);
    }
    RetInstruction *CreateRet(OperandType type, VReg vreg) {
        CREATE_INST(RetInstruction, type, vreg);
    }
    PhiInstruction *CreatePhi(OperandType type, VReg vdest, VReg vreg1, VReg vreg2) {
        CREATE_INST(PhiInstruction, type, vdest, vreg1, vreg2);
    }

#undef CREATE_INST

    void Clear() noexcept {
        for (auto *instr : instrs) {
            delete instr;
        }
        instrs.clear();
    }

private:
    std::vector<InstructionBase *> instrs;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_
