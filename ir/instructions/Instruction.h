#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_

#include "AllocatorUtils.h"
#include <array>
#include "Concepts.h"
#include <cstdint>
#include "InstructionBase.h"
#include "Input.h"
#include "macros.h"
#include <span>
#include "Types.h"
#include <vector>


namespace ir {
using FunctionId = size_t;
static inline constexpr FunctionId INVALID_FUNCTION_ID = static_cast<FunctionId>(-1);

enum class CondCode {
    EQ,
    NE,
    LT,
    GE
};

class InputsInstruction: public InstructionBase {
public:
    InputsInstruction(Opcode opcode, OperandType type, std::pmr::memory_resource *memResource)
        : InstructionBase(opcode, type, memResource) {}
    virtual DEFAULT_DTOR(InputsInstruction);

    // TODO: add `GetInputs` method
    virtual size_t GetInputsCount() const = 0;
    virtual Input &GetInput(size_t idx) = 0;
    virtual const Input &GetInput(size_t idx) const = 0;
    virtual void SetInput(Input newInput, size_t idx) = 0;
    virtual void ReplaceInput(const Input &oldInput, Input newInput) = 0;

protected:
    void dumpImpl(log4cpp::CategoryStream &stream) const override {
        InstructionBase::dumpImpl(stream);
        for (size_t i = 0, end = GetInputsCount(); i < end; ++i) {
            stream << " #" << GetInput(i)->GetId();
        }
    }
};

template <int InputsNum>
class FixedInputsInstruction: public InputsInstruction {
public:
    FixedInputsInstruction(Opcode opcode, OperandType type, std::pmr::memory_resource *memResource)
        : InputsInstruction(opcode, type, memResource) {}

    template <IsSameType<Input>... T>
    FixedInputsInstruction(Opcode opcode, OperandType type, std::pmr::memory_resource *memResource,
                           T... ins)
        : InputsInstruction(opcode, type, memResource), inputs{ins...}
    {
        for (auto &it : inputs) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    size_t GetInputsCount() const override {
        return InputsNum;
    }
    Input &GetInput(size_t idx) override {
        return inputs.at(idx);
    }
    const Input &GetInput(size_t idx) const override {
        return inputs.at(idx);
    }
    void SetInput(Input newInput, size_t idx) override {
        inputs.at(idx) = newInput;
        if (newInput.GetInstruction()) {
            newInput->AddUser(this);
        }
    }
    void ReplaceInput(const Input &oldInput, Input newInput) override {
        auto iter = std::find(inputs.begin(), inputs.end(), oldInput);
        ASSERT(iter != inputs.end());
        *iter = newInput;
    }

    std::array<Input, InputsNum> &GetInputs() {
        // TODO: return span?
        return inputs;
    }
    auto GetInputs() const {
        // TODO: return array?
        return std::span{inputs};
    }

private:
    std::array<Input, InputsNum> inputs;
};

template <>
class FixedInputsInstruction<1>: public InputsInstruction {
public:
    FixedInputsInstruction(Opcode opcode, OperandType type, std::pmr::memory_resource *memResource)
        : InputsInstruction(opcode, type, memResource) {}
    FixedInputsInstruction(Opcode opcode, OperandType type, Input input, std::pmr::memory_resource *memResource)
        : InputsInstruction(opcode, type, memResource), input(input)
    {
        if (input.GetInstruction()) {
            input->AddUser(this);
        }
    }

    size_t GetInputsCount() const override {
        return 1;
    }
    Input &GetInput() {
        return input;
    }
    const Input &GetInput() const {
        return input;
    }
    Input &GetInput(size_t idx) override {
        ASSERT(idx == 0);
        return input;
    }
    const Input &GetInput(size_t idx) const override {
        ASSERT(idx == 0);
        return input;
    }
    void SetInput(Input newInput, size_t idx) override {
        ASSERT(idx == 0);
        input = newInput;
        if (input.GetInstruction()) {
            input->AddUser(this);
        }
    }
    void ReplaceInput(const Input &oldInput, Input newInput) override {
        ASSERT(input == oldInput);
        input = newInput;
    }

private:
    Input input;
};

template <typename T>
concept AllowedInputType = (IsSameType<Input, std::remove_cv_t<T>>
    || (std::is_pointer_v<T>
        && std::is_base_of_v<InstructionBase, std::remove_cv_t<std::remove_pointer_t<T>>>));

class VariableInputsInstruction: public InputsInstruction {
public:
    VariableInputsInstruction(Opcode opcode, OperandType type, std::pmr::memory_resource *memResource)
        : InputsInstruction(opcode, type, memResource),
          inputs(memResource) {}

    // TODO: specify correct concept
    template <typename Ins>
    VariableInputsInstruction(Opcode opcode, OperandType type, Ins ins,
                              std::pmr::memory_resource *memResource)
    requires AllowedInputType<typename Ins::value_type>
        : InputsInstruction(opcode, type, memResource),
          inputs(ins.begin(), ins.end(), memResource)
    {
        for (auto &it : inputs) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    // TODO: try generalizing constructor in respect to `ins` argument (with type-hints)
    template <typename Ins>
    VariableInputsInstruction(Opcode opcode, OperandType type, std::initializer_list<Ins> ins,
                              std::pmr::memory_resource *memResource)
    requires AllowedInputType<Ins>
        : InputsInstruction(opcode, type, memResource),
          inputs(ins.begin(), ins.end(), memResource)
    {
        for (auto &it : inputs) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    template <typename Ins, typename AllocatorT>
    VariableInputsInstruction(Opcode opcode, OperandType type, std::vector<Ins, AllocatorT> ins,
                              std::pmr::memory_resource *memResource)
    requires AllowedInputType<Ins>
        : InputsInstruction(opcode, type, memResource),
          inputs(ins.begin(), ins.end(), memResource)
    {
        for (auto &it : inputs) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    DEFAULT_DTOR(VariableInputsInstruction);

    size_t GetInputsCount() const override {
        return inputs.size();
    }
    Input &GetInput(size_t idx) override {
        return inputs.at(idx);
    }
    const Input &GetInput(size_t idx) const override {
        return inputs.at(idx);
    }
    void SetInput(Input newInput, size_t idx) override {
        inputs.at(idx) = newInput;
        if (newInput.GetInstruction()) {
            newInput->AddUser(this);
        }
    }
    void ReplaceInput(const Input &oldInput, Input newInput) override {
        auto iter = std::find(inputs.begin(), inputs.end(), oldInput);
        ASSERT(iter != inputs.end());
        *iter = newInput;
    }

    std::pmr::vector<Input> &GetInputs() {
        return inputs;
    }
    const std::pmr::vector<Input> &GetInputs() const {
        return inputs;
    }
    void AddInput(Input newInput) {
        inputs.push_back(newInput);
        if (newInput.GetInstruction()) {
            newInput->AddUser(this);
        }
    }

protected:
    std::pmr::vector<Input> inputs;
};

template <Numeric T>
class ImmediateMixin {
public:
    using Type = T;

    ImmediateMixin(T value) : value(value) {}

    auto GetValue() const {
        return value;
    }
    void SetValue(T new_value) {
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

// Specific instructions classes
class UnaryRegInstruction : public FixedInputsInstruction<1> {
public:
    UnaryRegInstruction(Opcode opcode, OperandType type, Input input, std::pmr::memory_resource *memResource)
        : FixedInputsInstruction(opcode, type, input, memResource) {}

    UnaryRegInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class BinaryRegInstruction : public FixedInputsInstruction<2> {
public:
    BinaryRegInstruction(Opcode opcode, OperandType type, Input in1, Input in2,
                         std::pmr::memory_resource *memResource)
        : FixedInputsInstruction(opcode, type, memResource, in1, in2) {}

    BinaryRegInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class ConstantInstruction : public InstructionBase, public ImmediateMixin<uint64_t> {
public:
    ConstantInstruction(Opcode opcode, OperandType type, std::pmr::memory_resource *memResource)
        : InstructionBase(opcode, type, memResource), ImmediateMixin<uint64_t>(0) {}
    ConstantInstruction(Opcode opcode, OperandType type, uint64_t value, std::pmr::memory_resource *memResource)
        : InstructionBase(opcode, type, memResource), ImmediateMixin<uint64_t>(value) {}

    ConstantInstruction *Copy(BasicBlock *targetBBlock) const override;

protected:
    void dumpImpl(log4cpp::CategoryStream &stream) const override {
        InstructionBase::dumpImpl(stream);
        stream << ' ' << GetValue();
    }
};

class BinaryImmInstruction : public FixedInputsInstruction<1>, public ImmediateMixin<uint64_t> {
public:
    BinaryImmInstruction(Opcode opcode, OperandType type, Input input, uint64_t imm,
                         std::pmr::memory_resource *memResource)
        : FixedInputsInstruction<1>(opcode, type, input, memResource),
          ImmediateMixin<uint64_t>(imm)
    {}

    BinaryImmInstruction *Copy(BasicBlock *targetBBlock) const override;

protected:
    void dumpImpl(log4cpp::CategoryStream &stream) const override {
        InputsInstruction::dumpImpl(stream);
        stream << ' ' << GetValue();
    }
};

class CompareInstruction : public FixedInputsInstruction<2>, public ConditionMixin {
public:
    CompareInstruction(Opcode opcode, OperandType type, CondCode ccode, Input in1, Input in2,
                       std::pmr::memory_resource *memResource)
        : FixedInputsInstruction(opcode, type, memResource, in1, in2),
          ConditionMixin(ccode)
    {}

    CompareInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class CastInstruction : public FixedInputsInstruction<1> {
public:
    CastInstruction(OperandType fromType, OperandType toType, Input input,
                    std::pmr::memory_resource *memResource)
        : FixedInputsInstruction(Opcode::CAST, fromType, input, memResource),
          toType(toType)
    {}

    auto GetTargetType() const {
        return toType;
    }
    void SetTargetType(OperandType newType) {
        toType = newType;
    }

    CastInstruction *Copy(BasicBlock *targetBBlock) const override;

private:
    OperandType toType;
};

class JumpInstruction : public InstructionBase {
public:
    JumpInstruction(Opcode opcode, std::pmr::memory_resource *memResource)
        : InstructionBase(
            opcode,
            OperandType::I64,
            memResource,
            InstructionBase::INVALID_ID,
            utils::underlying_logic_or(InstrProp::CF, InstrProp::SIDE_EFFECTS))
    {}

    BasicBlock *GetDestination();

    JumpInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class CondJumpInstruction : public InstructionBase {
public:
    CondJumpInstruction(std::pmr::memory_resource *memResource)
        : InstructionBase(
            Opcode::JCMP,
            OperandType::I64,
            memResource,
            InstructionBase::INVALID_ID,
            utils::underlying_logic_or(InstrProp::CF, InstrProp::SIDE_EFFECTS))
    {}

    BasicBlock *GetTrueDestination();

    BasicBlock *GetFalseDestination();

    CondJumpInstruction *Copy(BasicBlock *targetBBlock) const override;

private:
    // true branch must always be the first successor, false branch - the second
    template <int CmpRes>
    BasicBlock *getBranchDestinationImpl();
};

class RetInstruction : public FixedInputsInstruction<1> {
public:
    RetInstruction(OperandType type, Input input, std::pmr::memory_resource *memResource)
        : FixedInputsInstruction<1>(Opcode::RET, type, input, memResource) {}

    RetInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class RetVoidInstruction : public InstructionBase {
public:
    RetVoidInstruction(std::pmr::memory_resource *memResource)
        : InstructionBase(
            Opcode::RETVOID,
            OperandType::VOID,
            memResource,
            InstructionBase::INVALID_ID,
            utils::underlying_logic_or(InstrProp::CF, InstrProp::SIDE_EFFECTS))
    {}

    RetVoidInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class PhiInstruction : public VariableInputsInstruction {
public:
    PhiInstruction(OperandType type, std::pmr::memory_resource *memResource)
        : VariableInputsInstruction(Opcode::PHI, type, memResource),
          sourceBBlocks(memResource) {}

    template <typename Ins, typename Sources>
    PhiInstruction(OperandType type, Ins input, Sources sources, std::pmr::memory_resource *memResource)
    requires std::is_same_v<std::remove_cv_t<typename Sources::value_type>, BasicBlock *>
             && AllowedInputType<typename Ins::value_type>
        : VariableInputsInstruction(Opcode::PHI, type, input, memResource),
          sourceBBlocks(sources.cbegin(), sources.cend(), memResource)
    {
        ASSERT(inputs.size() == sourceBBlocks.size());
    }

    template <typename Ins, typename Sources>
    PhiInstruction(OperandType type, std::initializer_list<Ins> input, std::initializer_list<Sources> sources,
                   std::pmr::memory_resource *memResource)
    requires std::is_same_v<std::remove_cv_t<Sources>, BasicBlock *> && AllowedInputType<Ins>
        : VariableInputsInstruction(Opcode::PHI, type, input, memResource),
          sourceBBlocks(sources.begin(), sources.end(), memResource)
    {
        ASSERT(inputs.size() == sourceBBlocks.size());
    }

    std::pmr::vector<BasicBlock *> &GetSourceBasicBlocks() {
        return sourceBBlocks;
    }
    const std::pmr::vector<BasicBlock *> &GetSourceBasicBlocks() const {
        return sourceBBlocks;
    }
    BasicBlock *GetSourceBasicBlock(size_t idx) {
        return sourceBBlocks.at(idx);
    }
    const BasicBlock *GetSourceBasicBlock(size_t idx) const {
        return sourceBBlocks.at(idx);
    }
    void SetSourceBasicBlock(BasicBlock *bblock, size_t idx) {
        ASSERT(bblock);
        sourceBBlocks.at(idx) = bblock;
    }

    void AddPhiInput(Input newInput, BasicBlock *inputSource) {
        ASSERT(inputSource);
        AddInput(newInput);
        sourceBBlocks.push_back(inputSource);
    }

    PhiInstruction *Copy(BasicBlock *targetBBlock) const override;

private:
    std::pmr::vector<BasicBlock *> sourceBBlocks;
};

class InputArgumentInstruction : public InstructionBase {
public:
    InputArgumentInstruction(OperandType type, std::pmr::memory_resource *memResource)
        : InstructionBase(Opcode::ARG, type, memResource) {}

    InputArgumentInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class CallInstruction : public VariableInputsInstruction {
public:
    CallInstruction(OperandType type, FunctionId target, std::pmr::memory_resource *memResource)
        : VariableInputsInstruction(Opcode::CALL, type, memResource), callTarget(target) {}

    template <typename InputsType>
    CallInstruction(OperandType type,
                    FunctionId target,
                    InputsType input,
                    std::pmr::memory_resource *memResource)
        : VariableInputsInstruction(Opcode::CALL, type, input, memResource),
          callTarget(target)
    {}

    FunctionId GetCallTarget() const {
        return callTarget;
    }
    void SetCallTarget(FunctionId newTarget) {
        callTarget = newTarget;
    }

    CallInstruction *Copy(BasicBlock *targetBBlock) const override;

protected:
    void dumpImpl(log4cpp::CategoryStream &stream) const {
        InputsInstruction::dumpImpl(stream);
        stream << " (to " << GetCallTarget() << ')';
    }

private:
    // TODO: add callee function resolution?
    FunctionId callTarget;
};

class LoadInstruction : public InstructionBase, public ImmediateMixin<uint64_t> {
public:
    LoadInstruction(OperandType type, uint64_t addr, std::pmr::memory_resource *memResource)
        : InstructionBase(Opcode::LOAD, type, memResource),
          ImmediateMixin<uint64_t>(addr) {}

    LoadInstruction *Copy(BasicBlock *targetBBlock) const override;

protected:
    void dumpImpl(log4cpp::CategoryStream &stream) const override {
        InstructionBase::dumpImpl(stream);
        stream << ' ' << GetValue();
    }
};

class StoreInstruction : public FixedInputsInstruction<1>, public ImmediateMixin<uint64_t> {
public:
    StoreInstruction(Input storedValue, uint64_t addr, std::pmr::memory_resource *memResource)
        : FixedInputsInstruction<1>(Opcode::STORE, storedValue->GetType(), storedValue, memResource),
          ImmediateMixin<uint64_t>(addr) {}

    StoreInstruction *Copy(BasicBlock *targetBBlock) const override;

protected:
    void dumpImpl(log4cpp::CategoryStream &stream) const override {
        InputsInstruction::dumpImpl(stream);
        stream << ' ' << GetValue();
    }
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
