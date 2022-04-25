#ifndef __sysycompiler_backend_operand_h__
#define __sysycompiler_backend_operand_h__

#include <cstdint>
#include <string>

namespace backend {

class Operand {
  public:
    enum OperandKind { kReg, kImm, kLabel };
    const OperandKind kind;

    explicit Operand(const OperandKind kind) : kind(kind) {}
    virtual ~Operand() = default;
    Operand(const Operand &) = delete;
    Operand &operator=(const Operand &) = delete;
    Operand(Operand &&) = delete;
    Operand &operator=(Operand &&) = delete;

    virtual std::string Str() const = 0;

    template <typename T>
    T &Cast() {
        return dynamic_cast<T &>(*this);
    }
};

class RegOperand final : public Operand {
  public:
    enum { kFp = 11, kIp, kSp, kLr, kPc, kCpsr };

    explicit RegOperand(const int id) : Operand(kReg), id(id) { CheckId(); }

    bool IsVirtual() const { return id > kCpsr; }
    bool IsSpecial() const { return id >= kFp && id <= kCpsr; }

    std::string Str() const override;

  private:
    const int id;

    void CheckId() const;
};

class ImmOperand final : public Operand {
  public:
    explicit ImmOperand(const std::int32_t value)
        : Operand(kImm)
        , value(value)
        , isImm8m(CheckImm8m())
        , isImm16(CheckImm16m()) {}

    int GetValue() const { return value; }

    std::string Str() const override { return '#' + std::to_string(value); }

    bool IsImm8m() const { return isImm8m; }
    bool IsImm16() const { return isImm16; }

  private:
    const std::int32_t value;
    // A kind of #<imm32> that can be generated through
    // "#<imm8> ror #<imm4>*2"
    // TODO(neatlii): why ~(#<imm8> ror #<imm4>*2) can also pass
    const bool isImm8m;
    const bool isImm16;

    bool CheckImm8m() const;
    bool CheckImm16m() const;
};

class LabelOperand final : public Operand {
  public:
    explicit LabelOperand(std::string name)
        : Operand(kLabel), name(std::move(name)) {}

    const std::string &GetName() const { return name; }

    std::string Str() const override { return name; }

  private:
    const std::string name;
};

}  // namespace backend

#endif
