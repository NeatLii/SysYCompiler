#ifndef __sysycompiler_ir_type_h__
#define __sysycompiler_ir_type_h__

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ir {

/* declarations */

class Type;
class VoidType;
class FuncType;
class IntType;
class PtrType;
class LabelType;
class ArrayType;

/* definitions */

class Type {
  public:
    enum TypeKind { kVoid, kFunc, kInt, kPtr, kLabel, kArray };
    const TypeKind kind;

    explicit Type(const TypeKind kind) : kind(kind) {}
    virtual ~Type() = default;
    Type(const Type &) = delete;
    Type &operator=(const Type &) = delete;
    Type(Type &&) = delete;
    Type &operator=(Type &&) = delete;

    virtual std::string Str() const = 0;

    template <typename T>
    const T &Cast() const {
        return dynamic_cast<const T &>(*this);
    }
};

class VoidType final : public Type {
  public:
    VoidType() : Type(kVoid) {}

    std::string Str() const override { return "void"; }
};

class FuncType final : public Type {
  public:
    FuncType(Type *ret_type, const std::vector<Type *> &param_list);
    FuncType(Type *ret_type, std::vector<std::shared_ptr<Type>> param_list)
        : Type(kFunc), ret_type(ret_type), param_list(std::move(param_list)) {}

    const Type &GetRetType() const { return *ret_type; }

    const std::vector<std::shared_ptr<Type>> &GetParamList() const {
        return param_list;
    }
    std::vector<std::shared_ptr<Type>>::size_type GetParamNum() const {
        return param_list.size();
    }
    const Type &GetParamAt(
        const std::vector<std::shared_ptr<Type>>::size_type index) const {
        return *param_list[index];
    }

    std::string RetTypeStr() const;
    std::string ParamListStr() const;
    std::string ParamListWithNameStr() const;
    std::string Str() const override;

  private:
    const std::unique_ptr<Type> ret_type;
    std::vector<std::shared_ptr<Type>> param_list;
};

class IntType final : public Type {
  public:
    enum Width { kI1, kI32 };

    explicit IntType(const Width width) : Type(kInt), width(width) {}

    Width GetWidth() const { return width; }

    std::string Str() const override;

  private:
    const Width width;
};

class PtrType final : public Type {
  public:
    PtrType() : Type(kPtr), pointee(new IntType(IntType::kI32)) {}
    explicit PtrType(Type *pointee) : Type(kPtr), pointee(pointee) {}
    explicit PtrType(std::shared_ptr<Type> pointee)
        : Type(kPtr), pointee(std::move(pointee)) {}

    const Type &GetPointee() const { return *pointee; }
    std::shared_ptr<Type> GetPointeePtr() const { return pointee; }

    std::string Str() const override { return pointee->Str() + '*'; }

  private:
    const std::shared_ptr<Type> pointee;
};

class LabelType final : public Type {
  public:
    LabelType() : Type(kLabel) {}

    std::string Str() const override { return "label"; }
};

class ArrayType final : public Type {
  public:
    explicit ArrayType(std::vector<int> arr_dim_list)
        : Type(kArray), arr_dim_list(std::move(arr_dim_list)) {}

    const std::vector<int> &GetArrDimList() const { return arr_dim_list; }
    std::vector<int>::size_type GetArrDimNum() const {
        return arr_dim_list.size();
    }
    int GetArrDimAt(const std::vector<int>::size_type index) {
        return arr_dim_list[index];
    }

    std::string Str() const override;

  private:
    const std::vector<int> arr_dim_list;
};

}  // namespace ir

#endif
