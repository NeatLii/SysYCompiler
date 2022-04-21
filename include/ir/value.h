#ifndef __sysycompiler_ir_value_h__
#define __sysycompiler_ir_value_h__

#include <memory>
#include <string>
#include <utility>

#include "ir/type.h"

namespace ir {

/* declarations */

class Value;
class Imm;
class Var;

class GlobalVar;
class LocalVar;
class TmpVar;

/* definitions */

class Value {
  public:
    enum ValueKind { kImm, kVar, kGlobalVar, kLocalVar, kTmpVar };
    const ValueKind kind;

    Value(const ValueKind kind, Type *type) : kind(kind), type(type) {}
    Value(const ValueKind kind, std::shared_ptr<Type> type)
        : kind(kind), type(std::move(type)) {}

    virtual ~Value() = default;
    Value(const Value &) = delete;
    Value &operator=(const Value &) = delete;
    Value(Value &&) = delete;
    Value &operator=(Value &&) = delete;

    const Type &GetType() const { return *type; }
    std::shared_ptr<Type> GetTypePtr() const { return type; }

    virtual std::string Str() const = 0;
    virtual std::string TypeStr() const = 0;

    template <typename T>
    const T &Cast() const {
        return dynamic_cast<const T &>(*this);
    }

  protected:
    const std::shared_ptr<Type> type;
};

class Imm final : public Value {
  public:
    explicit Imm(const int value)
        : Value(kImm, new IntType(IntType::kI32)), value(value) {}

    int GetValue() const { return value; }

    std::string Str() const override { return std::to_string(value); }
    std::string TypeStr() const override {
        return "i32 " + std::to_string(value);
    }

  private:
    const int value;
};

class Var : public Value {
  public:
    Var(const ValueKind kind, Type *type, std::string name)
        : Value(kind, type), name(std::move(name)) {}
    Var(const ValueKind kind, std::shared_ptr<Type> type, std::string name)
        : Value(kind, std::move(type)), name(std::move(name)) {}

    const std::string &GetName() const { return name; }

  protected:
    const std::string name;
};

class GlobalVar final : public Var {
  public:
    GlobalVar(Type *type, std::string name)
        : Var(kGlobalVar, type, std::move(name)) {}
    GlobalVar(std::shared_ptr<Type> type, std::string name)
        : Var(kGlobalVar, std::move(type), std::move(name)) {}

    std::string Str() const override { return '@' + name; }
    std::string TypeStr() const override { return type->Str() + " @" + name; }
};

class LocalVar : public Var {
  public:
    LocalVar(Type *type, std::string name, const ValueKind kind = kLocalVar)
        : Var(kind, type, std::move(name)) {}
    LocalVar(std::shared_ptr<Type> type,
             std::string name,
             const ValueKind kind = kLocalVar)
        : Var(kind, std::move(type), std::move(name)) {}

    std::string Str() const override { return '%' + name; }
    std::string TypeStr() const override { return type->Str() + " %" + name; }
};

class TmpVar final : public LocalVar {
  public:
    explicit TmpVar(const int num)
        : LocalVar(new IntType(IntType::kI32), std::to_string(num), kTmpVar)
        , id(num) {}
    TmpVar(Type *type, const int num)
        : LocalVar(type, std::to_string(num), kTmpVar), id(num) {}
    TmpVar(std::shared_ptr<Type> type, const int num)
        : LocalVar(std::move(type), std::to_string(num), kTmpVar), id(num) {}

    void SetID(const int id) { this->id = id; }
    int GetID() const { return id; }

  private:
    int id;
};

}  // namespace ir

#endif
