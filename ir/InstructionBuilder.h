#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_

#include "Concepts.h"
#include "Graph.h"
#include "Instruction.h"
#include "macros.h"
#include <vector>


namespace ir {
class InstructionBuilder {
public:
    explicit InstructionBuilder(ArenaAllocator *const allocator)
        : allocator(allocator), instrs(allocator->ToSTL()) {}
    NO_COPY_SEMANTIC(InstructionBuilder);
    NO_MOVE_SEMANTIC(InstructionBuilder);
    virtual DEFAULT_DTOR(InstructionBuilder);

    void PushBackInstruction(BasicBlock *bblock, InstructionBase *instr) {
        bblock->PushBackInstruction(instr);
    }
    template <typename... T>
    void PushBackInstruction(BasicBlock *bblock, InstructionBase *instr, T *... reminder)
    requires InstructionType<InstructionBase, T...> {
        bblock->PushBackInstruction(instr);
        PushBackInstruction(bblock, reminder...);
    }

    void PushForwardInstruction(BasicBlock *bblock, InstructionBase *instr) {
        bblock->PushForwardInstruction(instr);
    }
    template <typename... T>
    void PushForwardInstruction(BasicBlock *bblock, InstructionBase *instr, T *... reminder)
    requires InstructionType<InstructionBase, T...> {
        bblock->PushForwardInstruction(instr);
        PushForwardInstruction(bblock, reminder...);
    }

// TODO: implement and use Arena allocator
#define CREATE_INST(name, ...)                                  \
    auto *inst = allocator->template New<name>(__VA_ARGS__);    \
    instrs.push_back(inst);                                     \
    inst->SetId(instrs.size());                                 \
    return inst

    BinaryRegInstruction *CreateMul(OperandType type, Input in1, Input in2) {
        CREATE_INST(BinaryRegInstruction, Opcode::MUL, type, in1, in2);
    }
    template <ValidOpType T>
    BinaryImmInstruction *CreateAddi(OperandType type, Input input, T imm) {
        CREATE_INST(BinaryImmInstruction, Opcode::ADDI, type, input, imm);
    }
    // TODO: remove MOVI instruction
    template <ValidOpType T>
    ConstantInstruction *CreateConst(OperandType type, T imm) {
        CREATE_INST(ConstantInstruction, Opcode::CONST, type, imm);
    }
    CastInstruction *CreateCast(OperandType fromType, OperandType toType, Input input) {
        CREATE_INST(CastInstruction, fromType, toType, input);
    }
    CompareInstruction *CreateCmp(OperandType type, CondCode ccode, Input in1, Input in2) {
        CREATE_INST(CompareInstruction, Opcode::CMP, type, ccode, in1, in2);
    }
    JumpInstruction *CreateJa(int64_t imm = -1) {
        CREATE_INST(JumpInstruction, Opcode::JA, imm);
    }
    JumpInstruction *CreateJmp(int64_t imm) {
        CREATE_INST(JumpInstruction, Opcode::JMP, imm);
    }
    RetInstruction *CreateRet(OperandType type, Input input) {
        CREATE_INST(RetInstruction, type, input);
    }

    PhiInstruction *CreatePhi(OperandType type) {
        CREATE_INST(PhiInstruction, type, allocator);
    }
    template <typename Ins, typename Sources>
    PhiInstruction *CreatePhi(OperandType type, Ins inputs, Sources sources)
    requires std::is_same_v<std::remove_cv_t<typename Sources::value_type>, BasicBlock *>
             && AllowedInputType<typename Ins::value_type>
    {
        CREATE_INST(PhiInstruction, type, inputs, sources, allocator);
    }
    template <typename Ins, typename Sources>
    PhiInstruction *CreatePhi(OperandType type, std::initializer_list<Ins> inputs, std::initializer_list<Sources> sources)
    requires std::is_same_v<std::remove_cv_t<Sources>, BasicBlock *> && AllowedInputType<Ins>
    {
        CREATE_INST(PhiInstruction, type, inputs, sources, allocator);
    }

    InputArgumentInstruction *CreateArg(OperandType type) {
        CREATE_INST(InputArgumentInstruction, type);
    }

#undef CREATE_INST

private:
    ArenaAllocator *const allocator;
    utils::memory::ArenaVector<InstructionBase *> instrs;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_
