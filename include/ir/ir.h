#ifndef __sysycompiler_ir_ir_h__
#define __sysycompiler_ir_ir_h__

#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ir/type.h"
#include "ir/value.h"

namespace ir {

/* declarations */

class Inst;
class RetInst;
class BrInst;
class BinaryOpInst;
class BitwiseOpInst;
class AllocaInst;
class LoadInst;
class StoreInst;
class GetelementptrInst;
class ZextInst;
class BitcastInst;
class IcmpInst;
class PhiInst;
class CallInst;

class BasicBlock;

class GlobalVarDef;
class FuncDecl;
class FuncDef;

class Module;

/* definitions */

class Inst {
  public:
    enum InstKind {
        kRet,
        kBr,
        kBinaryOp,
        kBitwiseOp,
        kAlloca,
        kLoad,
        kStore,
        kGetelementptr,
        kZext,
        kBitcast,
        kIcmp,
        kPhi,
        kCall
    };
    const InstKind kind;

    explicit Inst(const InstKind kind) : kind(kind) {}
    virtual ~Inst() = default;
    Inst(const Inst &) = delete;
    Inst &operator=(const Inst &) = delete;
    Inst(Inst &&) = delete;
    Inst &operator=(Inst &&) = delete;

    bool IsTerminateInst() const { return kRet <= kind && kind <= kBr; }

    virtual std::string Str() const = 0;

    template <typename T>
    T &Cast() {
        return dynamic_cast<T &>(*this);
    }

    static void CheckType(const std::string &inst,
                          const std::shared_ptr<Value> &value,
                          Type::TypeKind kind,
                          IntType::Width width = IntType::kI32);

  protected:
    virtual void Check() const = 0;
};

// ret <type> <value>
// ret void
class RetInst final : public Inst {
  public:
    RetInst() : Inst(kRet) {}
    explicit RetInst(Value *ret) : Inst(kRet), ret(ret) { Check(); }
    explicit RetInst(std::shared_ptr<Value> ret)
        : Inst(kRet), ret(std::move(ret)) {
        Check();
    }

    bool HasRet() const { return ret != nullptr; }

    void SetRet(Value *ret) { this->ret.reset(ret); }
    void SetRet(std::shared_ptr<Value> ret) { this->ret = std::move(ret); }
    const Value &GetRet() const { return *ret; }

    std::string Str() const override;

  private:
    std::shared_ptr<Value> ret;  // i32

    void Check() const override;
};

// br lable <dest>
// br i1 <cond>, lable <iftrue>, lable <iffalse>
class BrInst final : public Inst {
  public:
    explicit BrInst(Value *cond, int stuff) : Inst(kBr), cond(cond) {}
    explicit BrInst(std::shared_ptr<Value> cond, int stuff)
        : Inst(kBr), cond(std::move(cond)) {}

    explicit BrInst(Var *dest) : Inst(kBr), if_true(dest) { Check(); }
    explicit BrInst(std::shared_ptr<Var> dest)
        : Inst(kBr), if_true(std::move(dest)) {
        Check();
    }

    BrInst(Value *cond, Var *if_true, Var *if_false)
        : Inst(kBr), cond(cond), if_true(if_true), if_false(if_false) {
        Check();
    }
    BrInst(std::shared_ptr<Value> cond,
           std::shared_ptr<Var> if_true,
           std::shared_ptr<Var> if_false)
        : Inst(kBr)
        , cond(std::move(cond))
        , if_true(std::move(if_true))
        , if_false(std::move(if_false)) {
        Check();
    }

    // dest = if_true
    bool HasDest() const { return cond == nullptr; }
    void SetDest(Var *dest) { SetTrue(dest); }
    void SetDest(std::shared_ptr<Var> dest) { SetTrue(std::move(dest)); }
    const Var &GetDest() const { return GetTrue(); }

    void SetCond(Value *cond) { this->cond.reset(cond); }
    void SetCond(std::shared_ptr<Value> cond) { this->cond = std::move(cond); }
    const Value &GetCond() const { return *cond; }

    void SetTrue(Var *if_true) { this->if_true.reset(if_true); }
    void SetTrue(std::shared_ptr<Var> if_true) {
        this->if_true = std::move(if_true);
    }
    const Var &GetTrue() const { return *if_true; }

    void SetFalse(Var *if_false) { this->if_false.reset(if_false); }
    void SetFalse(std::shared_ptr<Var> if_false) {
        this->if_false = std::move(if_false);
    }
    const Var &GetFalse() const { return *if_false; }

    std::string Str() const override;

  private:
    std::shared_ptr<Value> cond;    // i1
    std::shared_ptr<Var> if_true;   // label
    std::shared_ptr<Var> if_false;  // label

    void Check() const override;
};

// <result> = op <ty> <lhs>, <rhs>
class BinaryOpInst final : public Inst {
  public:
    enum BinaryOpKind { kAdd, kSub, kMul, kSDiv, kSRem };
    const BinaryOpKind op_code;

    BinaryOpInst(const BinaryOpKind op_code,
                 Var *result,
                 Value *lhs,
                 Value *rhs)
        : Inst(kBinaryOp)
        , op_code(op_code)
        , result(result)
        , lhs(lhs)
        , rhs(rhs) {
        Check();
    }
    BinaryOpInst(const BinaryOpKind op_code,
                 std::shared_ptr<Var> result,
                 std::shared_ptr<Value> lhs,
                 std::shared_ptr<Value> rhs)
        : Inst(kBinaryOp)
        , op_code(op_code)
        , result(std::move(result))
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {
        Check();
    }

    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetLHS(Value *lhs) { this->lhs.reset(lhs); }
    void SetLHS(std::shared_ptr<Value> lhs) { this->lhs = std::move(lhs); }
    const Value &GetLHS() const { return *lhs; }

    void SetRHS(Value *rhs) { this->rhs.reset(rhs); }
    void SetRHS(std::shared_ptr<Value> rhs) { this->rhs = std::move(rhs); }
    const Value &GetRHS() const { return *rhs; }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;  // i32
    std::shared_ptr<Value> lhs;   // i32
    std::shared_ptr<Value> rhs;   // i32

    void Check() const override;
};

// <result> = op <ty> <lhs>, <rhs>
class BitwiseOpInst final : public Inst {
  public:
    enum BitwiseOpKind { kAnd, kOr };
    const BitwiseOpKind op_code;

    BitwiseOpInst(const BitwiseOpKind op_code,
                  Var *result,
                  Value *lhs,
                  Value *rhs)
        : Inst(kBitwiseOp)
        , op_code(op_code)
        , result(result)
        , lhs(lhs)
        , rhs(rhs) {
        Check();
    }
    BitwiseOpInst(const BitwiseOpKind op_code,
                  std::shared_ptr<Var> result,
                  std::shared_ptr<Value> lhs,
                  std::shared_ptr<Value> rhs)
        : Inst(kBitwiseOp)
        , op_code(op_code)
        , result(std::move(result))
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {
        Check();
    }
    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetLHS(Value *lhs) { this->lhs.reset(lhs); }
    void SetLHS(std::shared_ptr<Value> lhs) { this->lhs = std::move(lhs); }
    const Value &GetLHS() const { return *lhs; }

    void SetRHS(Value *rhs) { this->rhs.reset(rhs); }
    void SetRHS(std::shared_ptr<Value> rhs) { this->rhs = std::move(rhs); }
    const Value &GetRHS() const { return *rhs; }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;  // i1
    std::shared_ptr<Value> lhs;   // i1
    std::shared_ptr<Value> rhs;   // i1

    void Check() const override;
};

// <result> = alloca <ty>
class AllocaInst final : public Inst {
  public:
    explicit AllocaInst(Var *result) : Inst(kAlloca), result(result) {
        Check();
    }
    explicit AllocaInst(std::shared_ptr<Var> result)
        : Inst(kAlloca), result(std::move(result)) {
        Check();
    }

    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;  // ptr

    void Check() const override;
};

// <result> = load <ty>, <ty>* <pointer>
class LoadInst final : public Inst {
  public:
    LoadInst(Var *result, Var *ptr) : Inst(kLoad), result(result), ptr(ptr) {
        Check();
    }
    LoadInst(std::shared_ptr<Var> result, std::shared_ptr<Var> ptr)
        : Inst(kLoad), result(std::move(result)), ptr(std::move(ptr)) {
        Check();
    }

    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetPtr(Var *ptr) { this->ptr.reset(ptr); }
    void SetPtr(std::shared_ptr<Var> ptr) { this->ptr = std::move(ptr); }
    const Var &GetPtr() const { return *ptr; }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;
    std::shared_ptr<Var> ptr;  // ptr

    void Check() const override;
};

// store <ty> <value>, <ty>* <pointer>
class StoreInst final : public Inst {
  public:
    StoreInst(Value *result, Value *ptr)
        : Inst(kStore), value(result), ptr(ptr) {
        Check();
    }
    StoreInst(std::shared_ptr<Value> result, std::shared_ptr<Value> ptr)
        : Inst(kStore), value(std::move(result)), ptr(std::move(ptr)) {
        Check();
    }

    void SetValue(Value *value) { this->value.reset(value); }
    void SetValue(std::shared_ptr<Value> value) {
        this->value = std::move(value);
    }
    const Value &GetValue() const { return *value; }

    void SetPtr(Value *ptr) { this->ptr.reset(ptr); }
    void SetPtr(std::shared_ptr<Value> ptr) { this->ptr = std::move(ptr); }
    const Value &GetPtr() const { return *ptr; }

    std::string Str() const override;

  private:
    std::shared_ptr<Value> value;
    std::shared_ptr<Value> ptr;  // ptr

    void Check() const override;
};

// <result> = getelementptr <ty>, <ty>* <ptrval>{, <ty> <idx>}*
class GetelementptrInst final : public Inst {
  public:
    GetelementptrInst(Var *result,
                      Var *ptr,
                      std::vector<std::shared_ptr<Value>> idx_list)
        : Inst(kGetelementptr)
        , result(result)
        , ptr(ptr)
        , idx_list(std::move(idx_list)) {
        Check();
    }
    GetelementptrInst(std::shared_ptr<Var> result,
                      std::shared_ptr<Var> ptr,
                      std::vector<std::shared_ptr<Value>> idx_list)
        : Inst(kGetelementptr)
        , result(std::move(result))
        , ptr(std::move(ptr))
        , idx_list(std::move(idx_list)) {
        Check();
    }

    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetPtr(Var *ptr) { this->ptr.reset(ptr); }
    void SetPtr(std::shared_ptr<Var> ptr) { this->ptr = std::move(ptr); }
    const Var &GetPtr() const { return *ptr; }

    const std::vector<std::shared_ptr<Value>> &GetIdxList() const {
        return idx_list;
    }
    std::vector<std::shared_ptr<Value>>::size_type GetIdxNum() const {
        return idx_list.size();
    }
    const std::shared_ptr<Value> &GetIdxAt(
        const std::vector<int>::size_type index) const {
        return idx_list[index];
    }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;  // ptr
    std::shared_ptr<Var> ptr;     // ptr
    const std::vector<std::shared_ptr<Value>> idx_list;

    void Check() const override;
};

// <result> = zext <ty> <value> to <ty2>
class ZextInst final : public Inst {
  public:
    ZextInst(Value *result, Value *value)
        : Inst(kZext), result(result), value(value) {
        Check();
    }
    ZextInst(std::shared_ptr<Value> result, std::shared_ptr<Value> value)
        : Inst(kZext), result(std::move(result)), value(std::move(value)) {
        Check();
    }

    void SetResult(Value *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Value> result) {
        this->result = std::move(result);
    }
    const Value &GetResult() const { return *result; }

    void SetValue(Value *value) { this->value.reset(value); }
    void SetValue(std::shared_ptr<Value> value) {
        this->value = std::move(value);
    }
    const Value &GetValue() const { return *value; }

    std::string Str() const override;

  private:
    std::shared_ptr<Value> result;  // i32
    std::shared_ptr<Value> value;   // i1

    void Check() const override;
};

// <result> = bitcast <ty> <value> to <ty2>
class BitcastInst final : public Inst {
  public:
    BitcastInst(Var *result, Var *value)
        : Inst(kBitcast), result(result), value(value) {}
    BitcastInst(std::shared_ptr<Var> result, std::shared_ptr<Var> value)
        : Inst(kBitcast), result(std::move(result)), value(std::move(value)) {}

    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetValue(Var *value) { this->value.reset(value); }
    void SetValue(std::shared_ptr<Var> value) {
        this->value = std::move(value);
    }
    const Var &GetValue() const { return *value; }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;
    std::shared_ptr<Var> value;

    void Check() const override {}
};

// <result> = icmp <cond> <ty> <op1>, <op2>
class IcmpInst final : public Inst {
  public:
    enum CmpKind { kEQ, kNE, kSGT, kSGE, kSLT, kSLE };
    const CmpKind op_code;

    IcmpInst(const CmpKind op_code, Var *result, Value *lhs, Value *rhs)
        : Inst(kIcmp), op_code(op_code), result(result), lhs(lhs), rhs(rhs) {
        Check();
    }
    IcmpInst(const CmpKind op_code,
             std::shared_ptr<Var> result,
             std::shared_ptr<Value> lhs,
             std::shared_ptr<Value> rhs)
        : Inst(kIcmp)
        , op_code(op_code)
        , result(std::move(result))
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {
        Check();
    }

    void SetResult(Var *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Var> result) {
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetLHS(Value *lhs) { this->lhs.reset(lhs); }
    void SetLHS(std::shared_ptr<Value> lhs) { this->lhs = std::move(lhs); }
    const Value &GetLHS() const { return *lhs; }

    void SetRHS(Value *rhs) { this->rhs.reset(rhs); }
    void SetRHS(std::shared_ptr<Value> rhs) { this->rhs = std::move(rhs); }
    const Value &GetRHS() const { return *rhs; }

    std::string Str() const override;

  private:
    std::shared_ptr<Var> result;  // i1
    std::shared_ptr<Value> lhs;
    std::shared_ptr<Value> rhs;

    void Check() const override;
};

// <result> = phi <ty> [<val0>, <label0>], ...
class PhiInst final : public Inst {
  public:
    struct PhiValue {
        std::shared_ptr<Value> value;  // i32
        std::shared_ptr<Var> label;    // label

        PhiValue(Value *value, Var *label) : value(value), label(label) {
            Check();
        }
        PhiValue(std::shared_ptr<Value> value, std::shared_ptr<Var> label)
            : value(std::move(value)), label(std::move(label)) {
            Check();
        }

        std::string Str() const;

      private:
        void Check() const;
    };

    PhiInst(Value *result, std::vector<PhiValue> value_list)
        : Inst(kPhi), result(result), value_list(std::move(value_list)) {
        Check();
    }

    void SetResult(Value *result) { this->result.reset(result); }
    void SetResult(std::shared_ptr<Value> result) {
        this->result = std::move(result);
    }
    const Value &GetResult() const { return *result; }

    const std::vector<PhiValue> &GetValueList() const { return value_list; }
    std::vector<PhiValue>::size_type GetValueNum() const {
        return value_list.size();
    }
    const PhiValue &GetValueAt(
        const std::vector<PhiValue>::size_type index) const {
        return value_list[index];
    }

    std::string Str() const override;

  private:
    std::shared_ptr<Value> result;     // i32
    std::vector<PhiValue> value_list;  // <operand, label>

    void Check() const override;
};

// <result> = call <ty> <fnptrval>(<function args>)
class CallInst final : public Inst {
  public:
    CallInst(Var *result,
             Var *func,
             std::vector<std::shared_ptr<Value>> param_list)
        : Inst(kCall)
        , has_ret(true)
        , result(result)
        , func(func)
        , param_list(std::move(param_list)) {
        Check();
    }
    CallInst(std::shared_ptr<Var> result,
             std::shared_ptr<Var> func,
             std::vector<std::shared_ptr<Value>> param_list)
        : Inst(kCall)
        , has_ret(true)
        , result(std::move(result))
        , func(std::move(func))
        , param_list(std::move(param_list)) {
        Check();
    }
    CallInst(std::shared_ptr<Var> func,
             std::vector<std::shared_ptr<Value>> param_list)
        : Inst(kCall)
        , has_ret(false)
        , func(std::move(func))
        , param_list(std::move(param_list)) {
        Check();
    }

    bool HasRet() const { return has_ret; }

    void SetResult(Var *result) {
        has_ret = true;
        this->result.reset(result);
    }
    void SetResult(std::shared_ptr<Var> result) {
        has_ret = true;
        this->result = std::move(result);
    }
    const Var &GetResult() const { return *result; }

    void SetFunc(Var *func) { this->func.reset(func); }
    void SetFunc(std::shared_ptr<Var> func) { this->func = std::move(func); }
    const Var &GetFunc() const { return *func; }

    const std::vector<std::shared_ptr<Value>> &GetParamList() const {
        return param_list;
    }
    std::vector<std::shared_ptr<Value>>::size_type GetParamNum() const {
        return param_list.size();
    }
    const std::shared_ptr<Value> &GetParamAt(
        const std::vector<std::shared_ptr<Value>>::size_type index) const {
        return param_list[index];
    }

    std::string Str() const override;

  private:
    bool has_ret;
    std::shared_ptr<Var> result;  // i32
    std::shared_ptr<Var> func;    // func
    std::vector<std::shared_ptr<Value>> param_list;

    void Check() const override;
};

class BasicBlock {
  public:
    explicit BasicBlock(Var *label) : label(label) {
        Inst::CheckType("BasicBlock", this->label, Type::kLabel);
    }
    explicit BasicBlock(std::shared_ptr<Var> label) : label(std::move(label)) {
        Inst::CheckType("BasicBlock", this->label, Type::kLabel);
    }

    void AddPredecessor(BasicBlock *predecessor) {
        predecessor_list.emplace_back(predecessor);
    }
    void AddPredecessor(std::shared_ptr<BasicBlock> predecessor) {
        predecessor_list.emplace_back(predecessor);
    }
    std::list<std::shared_ptr<BasicBlock>> &GetPredecessorList() {
        return predecessor_list;
    }
    std::list<std::shared_ptr<BasicBlock>>::size_type GetPredecessorNum()
        const {
        return predecessor_list.size();
    }

    void AddSuccessor(BasicBlock *successor) {
        predecessor_list.emplace_back(successor);
    }
    void AddSuccessor(std::shared_ptr<BasicBlock> successor) {
        successor_list.emplace_back(successor);
    }
    std::list<std::shared_ptr<BasicBlock>> &GetSuccessorList() {
        return successor_list;
    }
    std::list<std::shared_ptr<BasicBlock>>::size_type GetSuccessorNum() const {
        return successor_list.size();
    }

    void SetLabel(Var *label) { this->label.reset(label); }
    void SetLabel(std::shared_ptr<Var> label) {
        this->label = std::move(label);
    }
    const Var &GetLabel() const { return *label; }

    void AddInst(Inst *inst) { inst_list.emplace_back(inst); }
    void AddInst(std::shared_ptr<Inst> inst) { inst_list.emplace_back(inst); }
    std::list<std::shared_ptr<Inst>> &GetInstList() { return inst_list; }
    std::list<std::shared_ptr<Inst>>::size_type GetrInstNum() const {
        return inst_list.size();
    }

    void Dump(std::ostream &ostream, const std::string &indent) const;

  private:
    std::list<std::shared_ptr<BasicBlock>> predecessor_list;
    std::list<std::shared_ptr<BasicBlock>> successor_list;
    std::shared_ptr<Var> label;
    std::list<std::shared_ptr<Inst>> inst_list;
};

// @<GlobalVarName> = <global | constant> <Type> [<InitializerConstant>]
class GlobalVarDef {
  public:
    GlobalVarDef(Var *ident,
                 const bool is_const,
                 std::vector<std::shared_ptr<Imm>> init_list,
                 const bool is_zero_init)
        : ident(ident)
        , is_const(is_const)
        , init_list(std::move(init_list))
        , is_zero_init(is_zero_init) {}
    GlobalVarDef(std::shared_ptr<Var> ident,
                 const bool is_const,
                 std::vector<std::shared_ptr<Imm>> init_list,
                 const bool is_zero_init)
        : ident(std::move(ident))
        , is_const(is_const)
        , init_list(std::move(init_list))
        , is_zero_init(is_zero_init) {}

    const Var &GetIdent() const { return *ident; }

    bool IsConst() const { return is_const; }

    const std::vector<std::shared_ptr<Imm>> &GetInitList() const {
        return init_list;
    }
    std::vector<std::shared_ptr<Imm>>::size_type GetInitNum() const {
        return init_list.size();
    }
    const Imm &GetInitAt(
        const std::vector<std::shared_ptr<Imm>>::size_type index) const {
        return *init_list[index];
    }

    bool IsZeroInit() const { return is_zero_init; }

    void Dump(std::ostream &ostream) const;

  private:
    const std::shared_ptr<Var> ident;
    const bool is_const;
    const std::vector<std::shared_ptr<Imm>> init_list;
    const bool is_zero_init;
};

// declare <ResultType> @<FunctionName> ([argument list])
class FuncDecl {
  public:
    explicit FuncDecl(Var *ident) : ident(ident) {}
    explicit FuncDecl(std::shared_ptr<Var> ident) : ident(std::move(ident)) {}

    void Dump(std::ostream &ostream) const;

  private:
    const std::shared_ptr<Var> ident;  // func
};

// define <ResultType> @<FunctionName> ([argument list]) { ... }
class FuncDef {
  public:
    explicit FuncDef(Var *ident,
                     std::vector<std::shared_ptr<TmpVar>> param_list)
        : ident(ident), param_list(std::move(param_list)) {}
    explicit FuncDef(std::shared_ptr<Var> ident,
                     std::vector<std::shared_ptr<TmpVar>> param_list)
        : ident(std::move(ident)), param_list(std::move(param_list)) {}

    void AddBlock(BasicBlock *block) { block_list.emplace_back(block); }
    void AddBlock(std::shared_ptr<BasicBlock> block) {
        block_list.emplace_back(block);
    }
    const std::list<std::shared_ptr<BasicBlock>> &GetBlockList() const {
        return block_list;
    }
    std::list<std::shared_ptr<BasicBlock>>::size_type GetBlockNum() const {
        return block_list.size();
    }

    const std::vector<std::shared_ptr<TmpVar>> &GetParamList() const {
        return param_list;
    }

    void Dump(std::ostream &ostream) const;

  private:
    const std::shared_ptr<Var> ident;  // func
    const std::vector<std::shared_ptr<TmpVar>> param_list;
    std::list<std::shared_ptr<BasicBlock>> block_list;
};

class Module {
  public:
    Module() = default;

    void AddVar(GlobalVarDef *var) { var_list.emplace_back(var); }
    void AddVar(std::shared_ptr<GlobalVarDef> var) {
        var_list.emplace_back(var);
    }
    const std::list<std::shared_ptr<GlobalVarDef>> &GetVarList() const {
        return var_list;
    }
    std::list<std::shared_ptr<GlobalVarDef>>::size_type GetVarNum() const {
        return var_list.size();
    }

    void AddFuncDecl(FuncDecl *func) { func_decl_list.emplace_back(func); }
    void AddFuncDecl(std::shared_ptr<FuncDecl> func) {
        func_decl_list.emplace_back(func);
    }
    const std::list<std::shared_ptr<FuncDecl>> &GetFuncDeclList() const {
        return func_decl_list;
    }
    std::list<std::shared_ptr<FuncDecl>>::size_type GetFuncDeclNum() const {
        return func_decl_list.size();
    }

    void AddFuncDef(FuncDef *func) { func_def_list.emplace_back(func); }
    void AddFuncDef(std::shared_ptr<FuncDef> func) {
        func_def_list.emplace_back(func);
    }
    const std::list<std::shared_ptr<FuncDef>> &GetFuncDefList() const {
        return func_def_list;
    }
    std::list<std::shared_ptr<FuncDef>>::size_type GetFuncDefNum() const {
        return func_def_list.size();
    }

    void Dump(std::ostream &ostream) const;

  private:
    std::list<std::shared_ptr<GlobalVarDef>> var_list;
    std::list<std::shared_ptr<FuncDecl>> func_decl_list;
    std::list<std::shared_ptr<FuncDef>> func_def_list;
};

}  // namespace ir

#endif
