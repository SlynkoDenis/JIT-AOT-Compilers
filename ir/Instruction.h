#ifndef JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
#define JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_

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
enum class CondCode {
    EQ,
    NE,
    LT,
    GE
};

class InputsInstruction: public InstructionBase {
public:
    InputsInstruction(Opcode opcode, OperandType type)
        : InstructionBase(opcode, type) {}
    virtual DEFAULT_DTOR(InputsInstruction);

    virtual Input &GetInput(size_t idx) = 0;
    virtual const Input &GetInput(size_t idx) const = 0;
    virtual void SetInput(Input newInput, size_t idx) = 0;
};

template <int InputsNum>
class FixedInputsInstruction: public InputsInstruction {
public:
    FixedInputsInstruction(Opcode opcode, OperandType type)
        : InputsInstruction(opcode, type) {}

    template <IsSameType<Input>... T>
    FixedInputsInstruction(Opcode opcode, OperandType type, T... inputs)
        : InputsInstruction(opcode, type), inputs{inputs...} {}

    Input &GetInput(size_t idx) override {
        return inputs.at(idx);
    }
    const Input &GetInput(size_t idx) const override {
        return inputs.at(idx);
    }
    void SetInput(Input newInput, size_t idx) override {
        inputs.at(idx) = newInput;
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
    FixedInputsInstruction(Opcode opcode, OperandType type)
        : InputsInstruction(opcode, type) {}
    FixedInputsInstruction(Opcode opcode, OperandType type, Input input)
        : InputsInstruction(opcode, type), input(input) {}

    Input &GetInput() {
        return input;
    }
    const Input &GetInput() const {
        return input;
    }
    Input &GetInput(size_t idx) override {
        ASSERT(idx > 0);
        return input;
    }
    const Input &GetInput(size_t idx) const override {
        ASSERT(idx > 0);
        return input;
    }
    void SetInput(Input newInput, size_t idx) override {
        ASSERT(idx > 0);
        input = newInput;
    }

private:
    Input input;
};

template <typename T>
concept AllowedInputType = (IsSameType<Input, T>
    || (std::is_pointer_v<T> && std::is_base_of<InstructionBase, std::remove_pointer_t<T>>::value));

class VariableInputsInstruction: public InputsInstruction {
public:
    VariableInputsInstruction(Opcode opcode, OperandType type)
        : InputsInstruction(opcode, type) {}

    template <AllowedInputType... T>
    VariableInputsInstruction(Opcode opcode, OperandType type, T... ins)
        : InputsInstruction(opcode, type), inputs{ins...} {}

    template <std::ranges::range Ins>
    VariableInputsInstruction(Opcode opcode, OperandType type, Ins ins)
        : InputsInstruction(opcode, type), inputs(ins.cbegin(), ins.cend()) {}

    DEFAULT_DTOR(VariableInputsInstruction);

    Input &GetInput(size_t idx) override {
        return inputs.at(idx);
    }
    const Input &GetInput(size_t idx) const override {
        return inputs.at(idx);
    }
    void SetInput(Input newInput, size_t idx) override {
        inputs.at(idx) = newInput;
    }

    std::vector<Input> &GetInputs() {
        return inputs;
    }
    const std::vector<Input> &GetInputs() const {
        return inputs;
    }
    void AddInput(Input newInput) {
        inputs.push_back(newInput);
    }

private:
    std::vector<Input> inputs;
};

template <Numeric T>
class ImmediateMixin {
public:
    using Type = T;

    explicit ImmediateMixin(T value) : value(value) {}

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
    UnaryRegInstruction(Opcode opcode, OperandType type, Input input)
        : FixedInputsInstruction(opcode, type, input) {}
};

class BinaryRegInstruction : public FixedInputsInstruction<2> {
public:
    BinaryRegInstruction(Opcode opcode, OperandType type, Input in1, Input in2)
        : FixedInputsInstruction(opcode, type, in1, in2) {}
};

class ConstantInstruction : public InstructionBase, public ImmediateMixin<uint64_t> {
public:
    ConstantInstruction(Opcode opcode, OperandType type)
        : InstructionBase(opcode, type), ImmediateMixin<uint64_t>(0) {}
    ConstantInstruction(Opcode opcode, OperandType type, uint64_t value)
        : InstructionBase(opcode, type), ImmediateMixin<uint64_t>(value) {}
};

class BinaryImmInstruction : public FixedInputsInstruction<1>, public ImmediateMixin<uint64_t> {
public:
    BinaryImmInstruction(Opcode opcode, OperandType type, Input input, uint64_t imm)
        : FixedInputsInstruction<1>(opcode, type, input), ImmediateMixin<uint64_t>(imm) {}
};

class CompareInstruction : public FixedInputsInstruction<2>, public ConditionMixin {
public:
    CompareInstruction(Opcode opcode, OperandType type, CondCode ccode, Input in1, Input in2)
        : FixedInputsInstruction(opcode, type, in1, in2), ConditionMixin(ccode) {}
};

class CastInstruction : public FixedInputsInstruction<1> {
public:
    CastInstruction(OperandType fromType, OperandType toType, Input input)
        : FixedInputsInstruction(Opcode::CAST, fromType, input), toType(toType) {}

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

class RetInstruction : public FixedInputsInstruction<1> {
public:
    RetInstruction(OperandType type, Input input)
        : FixedInputsInstruction<1>(Opcode::RET, type, input) {}
};

class PhiInstruction : public VariableInputsInstruction {
public:
    explicit PhiInstruction(OperandType type)
        : VariableInputsInstruction(Opcode::PHI, type) {}

    template <AllowedInputType... T>
    PhiInstruction(OperandType type, T... input)
        : VariableInputsInstruction(Opcode::PHI, type, input...) {}

    template <std::ranges::range Ins>
    PhiInstruction(OperandType type, Ins input)
        : VariableInputsInstruction(Opcode::PHI, type, input) {}
};

class InputArgumentInstruction : public InstructionBase {
public:
    explicit InputArgumentInstruction(OperandType type)
        : InstructionBase(Opcode::ARG, type) {}
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_INSTRUCTION_H_
