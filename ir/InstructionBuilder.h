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

#define CREATE_FIXED_INST(name)                             \
    auto *inst = allocator->template New<name>(allocator);  \
    instrs.push_back(inst);                                 \
    inst->SetId(instrs.size());                             \
    return inst

#define CREATE_INST(name, ...)                                          \
    auto *inst = allocator->template New<name>(__VA_ARGS__, allocator); \
    instrs.push_back(inst);                                             \
    inst->SetId(instrs.size());                                         \
    return inst

#define CREATE_INST_WITH_PROP(name, prop, ...)                          \
    auto *inst = allocator->template New<name>(__VA_ARGS__, allocator); \
    instrs.push_back(inst);                                             \
    inst->SetId(instrs.size());                                         \
    inst->SetProperty(prop);                                            \
    return inst

#define CREATE_COMMUTABLE_ARITHM(opcode)                                                    \
    BinaryRegInstruction *Create##opcode(OperandType type, Input in1, Input in2) {          \
        auto prop = ARITHM | utils::to_underlying(InstrProp::COMMUTABLE);                   \
        CREATE_INST_WITH_PROP(BinaryRegInstruction, prop, Opcode::opcode, type, in1, in2);  \
    }

#define CREATE_IMM_INST(opcode)                                                                 \
    template <ValidOpType T>                                                                    \
    BinaryImmInstruction *Create##opcode(OperandType type, Input input, T imm) {                \
        CREATE_INST_WITH_PROP(BinaryImmInstruction, ARITHM, Opcode::opcode, type, input, imm);  \
    }

    CREATE_COMMUTABLE_ARITHM(AND)
    CREATE_COMMUTABLE_ARITHM(OR)
    CREATE_COMMUTABLE_ARITHM(XOR)
    CREATE_COMMUTABLE_ARITHM(ADD)
    CREATE_COMMUTABLE_ARITHM(MUL)

    BinaryRegInstruction *CreateSUB(OperandType type, Input in1, Input in2) {
        CREATE_INST_WITH_PROP(BinaryRegInstruction, ARITHM, Opcode::SUB, type, in1, in2);
    }
    BinaryRegInstruction *CreateDIV(OperandType type, Input in1, Input in2) {
        CREATE_INST_WITH_PROP(BinaryRegInstruction, ARITHM, Opcode::DIV, type, in1, in2);
    }
    BinaryRegInstruction *CreateSRA(OperandType type, Input in1, Input in2) {
        CREATE_INST_WITH_PROP(BinaryRegInstruction, ARITHM, Opcode::SRA, type, in1, in2);
    }
    BinaryRegInstruction *CreateSLA(OperandType type, Input in1, Input in2) {
        CREATE_INST_WITH_PROP(BinaryRegInstruction, ARITHM, Opcode::SLA, type, in1, in2);
    }
    BinaryRegInstruction *CreateSLL(OperandType type, Input in1, Input in2) {
        CREATE_INST_WITH_PROP(BinaryRegInstruction, ARITHM, Opcode::SLL, type, in1, in2);
    }

    CREATE_IMM_INST(ANDI)
    CREATE_IMM_INST(ORI)
    CREATE_IMM_INST(XORI)
    CREATE_IMM_INST(ADDI)
    CREATE_IMM_INST(SUBI)
    CREATE_IMM_INST(MULI)
    CREATE_IMM_INST(DIVI)
    CREATE_IMM_INST(SRAI)
    CREATE_IMM_INST(SLAI)
    CREATE_IMM_INST(SLLI)

    UnaryRegInstruction *CreateNOT(OperandType type, Input input) {
        CREATE_INST_WITH_PROP(UnaryRegInstruction, ARITHM, Opcode::NOT, type, input);
    }
    UnaryRegInstruction *CreateNEG(OperandType type, Input input) {
        CREATE_INST_WITH_PROP(UnaryRegInstruction, ARITHM, Opcode::NEG, type, input);
    }
    template <ValidOpType T>
    ConstantInstruction *CreateCONST(OperandType type, T imm) {
        CREATE_INST(ConstantInstruction, Opcode::CONST, type, imm);
    }
    CastInstruction *CreateCAST(OperandType fromType, OperandType toType, Input input) {
        CREATE_INST_WITH_PROP(CastInstruction, InstrProp::INPUT, fromType, toType, input);
    }
    CompareInstruction *CreateCMP(OperandType type, CondCode ccode, Input in1, Input in2) {
        CREATE_INST_WITH_PROP(CompareInstruction, InstrProp::INPUT, Opcode::CMP, type, ccode, in1, in2);
    }
    CondJumpInstruction *CreateJCMP() {
        CREATE_FIXED_INST(CondJumpInstruction);
    }
    JumpInstruction *CreateJMP() {
        CREATE_INST(JumpInstruction, Opcode::JMP);
    }
    RetInstruction *CreateRET(OperandType type, Input input) {
        CREATE_INST_WITH_PROP(
            RetInstruction,
            utils::underlying_logic_or(InstrProp::JUMP, InstrProp::INPUT),
            type,
            input);
    }

    CallInstruction *CreateCALL(OperandType type, FunctionID target) {
        CREATE_INST(CallInstruction, type, target);
    }
    template <AllowedInputType Ins>
    CallInstruction *CreateCALL(OperandType type, FunctionID target, std::initializer_list<Ins> input) {
        CREATE_INST(CallInstruction, type, target, input);
    }

    LoadInstruction *CreateLOAD(OperandType type, uint64_t addr) {
        CREATE_INST_WITH_PROP(LoadInstruction, InstrProp::MEM, type, addr);
    }
    StoreInstruction *CreateSTORE(Input storedValue, uint64_t addr) {
        CREATE_INST_WITH_PROP(StoreInstruction, InstrProp::MEM, storedValue, addr);
    }

    PhiInstruction *CreatePHI(OperandType type) {
        CREATE_INST_WITH_PROP(PhiInstruction, InstrProp::INPUT, type);
    }
    template <typename Ins, typename Sources>
    PhiInstruction *CreatePHI(OperandType type, Ins inputs, Sources sources)
    requires std::is_same_v<std::remove_cv_t<typename Sources::value_type>, BasicBlock *>
             && AllowedInputType<typename Ins::value_type>
    {
        CREATE_INST_WITH_PROP(PhiInstruction, InstrProp::INPUT, type, inputs, sources);
    }
    template <typename Ins, typename Sources>
    PhiInstruction *CreatePHI(
        OperandType type,
        std::initializer_list<Ins> inputs,
        std::initializer_list<Sources> sources)
    requires std::is_same_v<std::remove_cv_t<Sources>, BasicBlock *> && AllowedInputType<Ins>
    {
        CREATE_INST_WITH_PROP(PhiInstruction, InstrProp::INPUT, type, inputs, sources);
    }

    InputArgumentInstruction *CreateARG(OperandType type) {
        CREATE_INST(InputArgumentInstruction, type);
    }

#undef CREATE_INST
#undef CREATE_INST_WITH_PROP
#undef CREATE_COMMUTABLE_ARITHM
#undef CREATE_IMM_INST

private:
    static constexpr InstructionPropT ARITHM = utils::underlying_logic_or(InstrProp::ARITH, InstrProp::INPUT);

private:
    ArenaAllocator *const allocator;

    utils::memory::ArenaVector<InstructionBase *> instrs;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_BUILDER_H_
