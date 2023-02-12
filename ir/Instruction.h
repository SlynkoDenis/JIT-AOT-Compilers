#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_

#include "arena/ArenaAllocator.h"
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
    InputsInstruction(Opcode opcode, OperandType type, ArenaAllocator *const allocator)
        : InstructionBase(opcode, type, allocator) {}
    virtual DEFAULT_DTOR(InputsInstruction);

    // TODO: add `GetInputs` method
    virtual size_t GetInputsCount() const = 0;
    virtual Input &GetInput(size_t idx) = 0;
    virtual const Input &GetInput(size_t idx) const = 0;
    virtual void SetInput(Input newInput, size_t idx) = 0;
    virtual void ReplaceInput(const Input &oldInput, Input newInput) = 0;

    void Dump(utils::dumper::EventDumper *dumper) const override {
        InstructionBase::Dump(dumper);
        for (size_t i = 0, end = GetInputsCount(); i < end; ++i) {
            dumper->Dump<false>(" #", GetInput(i)->GetId());
        }
    }
};

template <int InputsNum>
class FixedInputsInstruction: public InputsInstruction {
public:
    FixedInputsInstruction(Opcode opcode, OperandType type, ArenaAllocator *const allocator)
        : InputsInstruction(opcode, type, allocator) {}

    template <IsSameType<Input>... T>
    FixedInputsInstruction(Opcode opcode, OperandType type, ArenaAllocator *const allocator,
                           T... ins)
        : InputsInstruction(opcode, type, allocator), inputs{ins...}
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
    FixedInputsInstruction(Opcode opcode, OperandType type, ArenaAllocator *const allocator)
        : InputsInstruction(opcode, type, allocator) {}
    FixedInputsInstruction(Opcode opcode, OperandType type, Input input, ArenaAllocator *const allocator)
        : InputsInstruction(opcode, type, allocator), input(input)
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
    VariableInputsInstruction(Opcode opcode, OperandType type, ArenaAllocator *const allocator)
        : InputsInstruction(opcode, type, allocator),
          inputs(allocator->ToSTL()) {}

    // TODO: specify correct concept
    template <typename Ins>
    VariableInputsInstruction(Opcode opcode, OperandType type, Ins ins,
                              ArenaAllocator *const allocator)
    requires AllowedInputType<typename Ins::value_type>
        : InputsInstruction(opcode, type, allocator),
          inputs(ins.begin(), ins.end(), allocator->ToSTL())
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
                              ArenaAllocator *const allocator)
    requires AllowedInputType<Ins>
        : InputsInstruction(opcode, type, allocator),
          inputs(ins.begin(), ins.end(), allocator->ToSTL())
    {
        for (auto &it : inputs) {
            if (it.GetInstruction()) {
                it->AddUser(this);
            }
        }
    }

    template <typename Ins, typename AllocatorT>
    VariableInputsInstruction(Opcode opcode, OperandType type, std::vector<Ins, AllocatorT> ins,
                              ArenaAllocator *const allocator)
    requires AllowedInputType<Ins>
        : InputsInstruction(opcode, type, allocator),
          inputs(ins.begin(), ins.end(), allocator->ToSTL())
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

    utils::memory::ArenaVector<Input> &GetInputs() {
        return inputs;
    }
    const utils::memory::ArenaVector<Input> &GetInputs() const {
        return inputs;
    }
    void AddInput(Input newInput) {
        inputs.push_back(newInput);
        if (newInput.GetInstruction()) {
            newInput->AddUser(this);
        }
    }

protected:
    utils::memory::ArenaVector<Input> inputs;
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
    UnaryRegInstruction(Opcode opcode, OperandType type, Input input, ArenaAllocator *const allocator)
        : FixedInputsInstruction(opcode, type, input, allocator) {}

    UnaryRegInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class BinaryRegInstruction : public FixedInputsInstruction<2> {
public:
    BinaryRegInstruction(Opcode opcode, OperandType type, Input in1, Input in2,
                         ArenaAllocator *const allocator)
        : FixedInputsInstruction(opcode, type, allocator, in1, in2) {}

    BinaryRegInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class ConstantInstruction : public InstructionBase, public ImmediateMixin<uint64_t> {
public:
    ConstantInstruction(Opcode opcode, OperandType type, ArenaAllocator *const allocator)
        : InstructionBase(opcode, type, allocator), ImmediateMixin<uint64_t>(0) {}
    ConstantInstruction(Opcode opcode, OperandType type, uint64_t value, ArenaAllocator *const allocator)
        : InstructionBase(opcode, type, allocator), ImmediateMixin<uint64_t>(value) {}

    ConstantInstruction *Copy(BasicBlock *targetBBlock) const override;

    void Dump(utils::dumper::EventDumper *dumper) const override {
        InstructionBase::Dump(dumper);
        dumper->Dump<false>(' ', GetValue());
    }
};

class BinaryImmInstruction : public FixedInputsInstruction<1>, public ImmediateMixin<uint64_t> {
public:
    BinaryImmInstruction(Opcode opcode, OperandType type, Input input, uint64_t imm,
                         ArenaAllocator *const allocator)
        : FixedInputsInstruction<1>(opcode, type, input, allocator),
          ImmediateMixin<uint64_t>(imm)
    {}

    BinaryImmInstruction *Copy(BasicBlock *targetBBlock) const override;

    void Dump(utils::dumper::EventDumper *dumper) const override {
        InputsInstruction::Dump(dumper);
        dumper->Dump<false>(' ', GetValue());
    }
};

class CompareInstruction : public FixedInputsInstruction<2>, public ConditionMixin {
public:
    CompareInstruction(Opcode opcode, OperandType type, CondCode ccode, Input in1, Input in2,
                       ArenaAllocator *const allocator)
        : FixedInputsInstruction(opcode, type, allocator, in1, in2),
          ConditionMixin(ccode)
    {}

    CompareInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class CastInstruction : public FixedInputsInstruction<1> {
public:
    CastInstruction(OperandType fromType, OperandType toType, Input input,
                    ArenaAllocator *const allocator)
        : FixedInputsInstruction(Opcode::CAST, fromType, input, allocator),
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
    JumpInstruction(Opcode opcode, ArenaAllocator *const allocator)
        : InstructionBase(
            opcode,
            OperandType::I64,
            allocator,
            InstructionBase::INVALID_ID,
            utils::to_underlying(InstrProp::CF))
    {}

    BasicBlock *GetDestination();

    JumpInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class CondJumpInstruction : public InstructionBase {
public:
    CondJumpInstruction(ArenaAllocator *const allocator)
        : InstructionBase(
            Opcode::JCMP,
            OperandType::I64,
            allocator,
            InstructionBase::INVALID_ID,
            utils::to_underlying(InstrProp::CF))
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
    RetInstruction(OperandType type, Input input, ArenaAllocator *const allocator)
        : FixedInputsInstruction<1>(Opcode::RET, type, input, allocator) {}

    RetInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class RetVoidInstruction : public InstructionBase {
public:
    RetVoidInstruction(ArenaAllocator *const allocator)
        : InstructionBase(
            Opcode::RETVOID,
            OperandType::VOID,
            allocator,
            InstructionBase::INVALID_ID,
            utils::to_underlying(InstrProp::CF))
    {}

    RetVoidInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class PhiInstruction : public VariableInputsInstruction {
public:
    PhiInstruction(OperandType type, ArenaAllocator *const allocator)
        : VariableInputsInstruction(Opcode::PHI, type, allocator),
          sourceBBlocks(allocator->ToSTL()) {}

    template <typename Ins, typename Sources>
    PhiInstruction(OperandType type, Ins input, Sources sources, ArenaAllocator *const allocator)
    requires std::is_same_v<std::remove_cv_t<typename Sources::value_type>, BasicBlock *>
             && AllowedInputType<typename Ins::value_type>
        : VariableInputsInstruction(Opcode::PHI, type, input, allocator),
          sourceBBlocks(sources.cbegin(), sources.cend(), allocator->ToSTL())
    {
        ASSERT(inputs.size() == sourceBBlocks.size());
    }

    template <typename Ins, typename Sources>
    PhiInstruction(OperandType type, std::initializer_list<Ins> input, std::initializer_list<Sources> sources,
                   ArenaAllocator *const allocator)
    requires std::is_same_v<std::remove_cv_t<Sources>, BasicBlock *> && AllowedInputType<Ins>
        : VariableInputsInstruction(Opcode::PHI, type, input, allocator),
          sourceBBlocks(sources.begin(), sources.end(), allocator->ToSTL())
    {
        ASSERT(inputs.size() == sourceBBlocks.size());
    }

    utils::memory::ArenaVector<BasicBlock *> &GetSourceBasicBlocks() {
        return sourceBBlocks;
    }
    const utils::memory::ArenaVector<BasicBlock *> &GetSourceBasicBlocks() const {
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
    utils::memory::ArenaVector<BasicBlock *> sourceBBlocks;
};

class InputArgumentInstruction : public InstructionBase {
public:
    InputArgumentInstruction(OperandType type, ArenaAllocator *const allocator)
        : InstructionBase(Opcode::ARG, type, allocator) {}

    InputArgumentInstruction *Copy(BasicBlock *targetBBlock) const override;
};

class CallInstruction : public VariableInputsInstruction {
public:
    CallInstruction(OperandType type, FunctionId target, ArenaAllocator *const allocator)
        : VariableInputsInstruction(Opcode::CALL, type, allocator),
          callTarget(target),
          isInlined(false)
    {}

    template <typename InputsType>
    CallInstruction(OperandType type,
                    FunctionId target,
                    InputsType input,
                    ArenaAllocator *const allocator)
        : VariableInputsInstruction(Opcode::CALL, type, input, allocator),
          callTarget(target)
    {}

    FunctionId GetCallTarget() const {
        return callTarget;
    }
    void SetCallTarget(FunctionId newTarget) {
        callTarget = newTarget;
    }
    bool IsInlined() const {
        return isInlined;
    }
    void SetIsInlined(bool inlined) {
        isInlined = inlined;
    }

    CallInstruction *Copy(BasicBlock *targetBBlock) const override;

    void Dump(utils::dumper::EventDumper *dumper) const {
        InputsInstruction::Dump(dumper);
        dumper->Dump<false>(" (to ", GetCallTarget(), ')');
    }

private:
    // TODO: add callee function resolution?
    FunctionId callTarget;
    bool isInlined;
};

class LoadInstruction : public InstructionBase, public ImmediateMixin<uint64_t> {
public:
    LoadInstruction(OperandType type, uint64_t addr, ArenaAllocator *const allocator)
        : InstructionBase(Opcode::LOAD, type, allocator),
          ImmediateMixin<uint64_t>(addr) {}

    LoadInstruction *Copy(BasicBlock *targetBBlock) const override;

    void Dump(utils::dumper::EventDumper *dumper) const override {
        InstructionBase::Dump(dumper);
        dumper->Dump<false>(' ', GetValue());
    }
};

class StoreInstruction : public FixedInputsInstruction<1>, public ImmediateMixin<uint64_t> {
public:
    StoreInstruction(Input storedValue, uint64_t addr, ArenaAllocator *const allocator)
        : FixedInputsInstruction<1>(Opcode::STORE, storedValue->GetType(), storedValue, allocator),
          ImmediateMixin<uint64_t>(addr) {}

    StoreInstruction *Copy(BasicBlock *targetBBlock) const override;

    void Dump(utils::dumper::EventDumper *dumper) const override {
        InputsInstruction::Dump(dumper);
        dumper->Dump<false>(' ', GetValue());
    }
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
