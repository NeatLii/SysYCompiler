#ifndef __sysycompiler_backend_instruction_h__
#define __sysycompiler_backend_instruction_h__

#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "backend/operand.h"

namespace backend {

/* declarations */

class Inst;

class InsMov;
class InsLdr;
class InsStr;
class InsPush;
class InsPop;

class InsCmp;
class InsB;
class InsBl;
class InsBx;

class InsAdd;
class InsSub;
class InsMul;
class InsSDiv;
class InsAnd;
class InsOrr;

class InsNop;
class InsLabel;

class GlobalVar;
class Function;
class Assembly;

/* definitions */

class Inst {
  public:
    enum InstKind {
        kInsMov,
        kInsLdr,
        kInsStr,
        kInsPush,
        kInsPop,

        kInsCmp,
        kInsB,
        kInsBl,
        kInsBx,

        kInsAdd,
        kInsSub,
        kInsMul,
        kInsSDiv,
        kInsAnd,
        kInsOrr,

        kInsNop,
        kInsLabel
    };
    const InstKind op;

    enum CondKind { kAL, kEQ, kNE, kGT, kGE, kLT, kLE };
    const CondKind cond;

    explicit Inst(const InstKind op, const CondKind cond = kAL)
        : op(op), cond(cond) {}

    virtual ~Inst() = default;
    Inst(const Inst &) = delete;
    Inst &operator=(const Inst &) = delete;
    Inst(Inst &&) = delete;
    Inst &operator=(Inst &&) = delete;

    virtual std::string Str() const = 0;

    template <typename T>
    T &Cast() {
        return dynamic_cast<T &>(*this);
    }

  protected:
    static const std::array<std::string, kInsLabel + 1> op_map;
    static const std::array<std::string, kLE + 1> cond_map;
};

const std::array<std::string, Inst::kInsLabel + 1> Inst::op_map
    = {"\tmov",  "\tldr", "\tstr", "\tpush", "\tpop", "\tcmp",
       "\tb",    "\tbl",  "\tbx",  "\tadd",  "\tsub", "\tmul",
       "\tsdiv", "\tand", "\torr", "\tnop",  ""};

const std::array<std::string, Inst::kLE + 1> Inst::cond_map
    = {"", "eq", "ne", "gt", "ge", "lt", "le"};

// mov{cond} Rd, Rm
// mov{cond} Rd, #<imm16>
// mov{cond} Rd, #<imm8m>
class InsMov final : public Inst {
  public:
    InsMov(std::shared_ptr<RegOperand> Rd,
           const std::shared_ptr<RegOperand> &Rm,
           const CondKind cond = kAL)
        : Inst(kInsMov, cond), Rd(std::move(Rd)), Rm_imm(Rm) {}
    InsMov(std::shared_ptr<RegOperand> Rd,
           const std::shared_ptr<ImmOperand> &imm,
           const CondKind cond = kAL)
        : Inst(kInsMov, cond), Rd(std::move(Rd)), Rm_imm(imm) {
        CheckImm();
    }

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<Operand> Rm_imm;

    void CheckImm() const;
};

// ldr{cond} Rd, [Rn]
// ldr{cond} Rd, [Rn, #<offset>]
// ldr{cond} Rd, =#<imm32>      @ pseudo-instruction
// ldr{cond} Rd, =label         @ pseudo-instruction
class InsLdr final : public Inst {
  public:
    InsLdr(std::shared_ptr<RegOperand> Rd,
           const std::shared_ptr<RegOperand> &Rn,
           const CondKind cond = kAL)
        : Inst(kInsLdr, cond), Rd(std::move(Rd)), Rn_imm_label(Rn) {}
    InsLdr(std::shared_ptr<RegOperand> Rd,
           const std::shared_ptr<RegOperand> &Rn,
           std::shared_ptr<ImmOperand> offset,
           const CondKind cond = kAL)
        : Inst(kInsLdr, cond)
        , Rd(std::move(Rd))
        , Rn_imm_label(Rn)
        , offset(std::move(offset)) {}
    InsLdr(std::shared_ptr<RegOperand> Rd,
           const std::shared_ptr<ImmOperand> &imm32,
           const CondKind cond = kAL)
        : Inst(kInsLdr, cond), Rd(std::move(Rd)), Rn_imm_label(imm32) {}
    InsLdr(std::shared_ptr<RegOperand> Rd,
           const std::shared_ptr<LabelOperand> &label,
           const CondKind cond = kAL)
        : Inst(kInsLdr, cond), Rd(std::move(Rd)), Rn_imm_label(label) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<Operand> Rn_imm_label;
    const std::shared_ptr<ImmOperand> offset;
};

// str{cond} Rd, [Rn]
// str{cond} Rd, [Rn, #<offset>]
class InsStr final : public Inst {
  public:
    InsStr(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const CondKind cond = kAL)
        : Inst(kInsStr, cond), Rd(std::move(Rd)), Rn(std::move(Rn)) {}
    InsStr(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           std::shared_ptr<ImmOperand> offset,
           const CondKind cond = kAL)
        : Inst(kInsStr, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , offset(std::move(offset)) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<ImmOperand> offset;
};

// push{cond} <reglist>
// @ note: <reglist> is a comma separated sequence of registers, enclosed in
// @ curly braces '{' and '}'
class InsPush final : public Inst {
  public:
    explicit InsPush(std::vector<std::shared_ptr<RegOperand>> reg_list,
                     const CondKind cond = kAL)
        : Inst(kInsPush, cond), reg_list(std::move(reg_list)) {}

    std::string Str() const override;

  private:
    const std::vector<std::shared_ptr<RegOperand>> reg_list;
};

// pop{cond} <reglist>
// @ note: <reglist> is a comma separated sequence of registers, enclosed in
// @ curly braces '{' and '}'
class InsPop final : public Inst {
  public:
    explicit InsPop(std::vector<std::shared_ptr<RegOperand>> reg_list,
                    const CondKind cond = kAL)
        : Inst(kInsPop, cond), reg_list(std::move(reg_list)) {}

    std::string Str() const override;

  private:
    const std::vector<std::shared_ptr<RegOperand>> reg_list;
};

// cmp{cond} Rn, Rm
// cmp{cond}, Rn, #<imm8m>
class InsCmp final : public Inst {
  public:
    InsCmp(std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<RegOperand> &Rm,
           const CondKind cond = kAL)
        : Inst(kInsCmp, cond), Rn(std::move(Rn)), Rm_imm(Rm) {}
    InsCmp(std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<ImmOperand> &imm8m,
           const CondKind cond = kAL)
        : Inst(kInsCmp, cond), Rn(std::move(Rn)), Rm_imm(imm8m) {
        CheckImm();
    }

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rm_imm;

    void CheckImm() const;
};

// b{cond} label
class InsB final : public Inst {
  public:
    explicit InsB(std::shared_ptr<LabelOperand> label,
                  const CondKind cond = kAL)
        : Inst(kInsB, cond), label(std::move(label)) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<LabelOperand> label;
};

// bl{cond} label(PLT)
class InsBl final : public Inst {
  public:
    explicit InsBl(std::shared_ptr<LabelOperand> label,
                   const CondKind cond = kAL)
        : Inst(kInsBl, cond), label(std::move(label)) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<LabelOperand> label;
};

// bx{cond} Rm
class InsBx final : public Inst {
  public:
    explicit InsBx(std::shared_ptr<RegOperand> Rm, const CondKind cond = kAL)
        : Inst(kInsBx, cond), Rm(std::move(Rm)) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rm;
};

// add{cond} Rd, Rn, Rm
// add{cond} Rd, Rn, #<imm8m>
class InsAdd final : public Inst {
  public:
    InsAdd(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<RegOperand> &Rm,
           const CondKind cond = kAL)
        : Inst(kInsAdd, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(Rm) {}
    InsAdd(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<ImmOperand> &imm8m,
           const CondKind cond = kAL)
        : Inst(kInsAdd, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(imm8m) {
        CheckImm();
    }

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rm_imm;

    void CheckImm() const;
};

// sub{cond} Rd, Rn, Rm
// sub{cond} Rd, Rn, #<imm8m>
class InsSub final : public Inst {
  public:
    InsSub(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<RegOperand> &Rm,
           const CondKind cond = kAL)
        : Inst(kInsSub, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(Rm) {}
    InsSub(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<ImmOperand> &imm8m,
           const CondKind cond = kAL)
        : Inst(kInsSub, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(imm8m) {
        CheckImm();
    }

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rm_imm;

    void CheckImm() const;
};

// mul{cond} Rd, Rm, Rs
class InsMul final : public Inst {
  public:
    InsMul(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<RegOperand> &Rs,
           const CondKind cond = kAL)
        : Inst(kInsMul, cond), Rd(std::move(Rd)), Rn(std::move(Rn)), Rs(Rs) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rs;
};

// sdiv{cond} Rd, Rn, Rm
class InsSDiv final : public Inst {
  public:
    InsSDiv(std::shared_ptr<RegOperand> Rd,
            std::shared_ptr<RegOperand> Rn,
            const std::shared_ptr<RegOperand> &Rs,
            const CondKind cond = kAL)
        : Inst(kInsSDiv, cond), Rd(std::move(Rd)), Rn(std::move(Rn)), Rs(Rs) {}

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rs;
};

// and{cond} Rd, Rn, Rm
// and{cond} Rd, Rn, #<imm8m>
class InsAnd final : public Inst {
  public:
    InsAnd(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<RegOperand> &Rm,
           const CondKind cond = kAL)
        : Inst(kInsAnd, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(Rm) {}
    InsAnd(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<ImmOperand> &imm8m,
           const CondKind cond = kAL)
        : Inst(kInsAnd, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(imm8m) {
        CheckImm();
    }

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rm_imm;

    void CheckImm() const;
};

// orr{cond} Rd, Rn, Rm
// orr{cond} Rd, Rn, #<imm8m>
class InsOrr final : public Inst {
  public:
    InsOrr(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<RegOperand> &Rm,
           const CondKind cond = kAL)
        : Inst(kInsOrr, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(Rm) {}
    InsOrr(std::shared_ptr<RegOperand> Rd,
           std::shared_ptr<RegOperand> Rn,
           const std::shared_ptr<ImmOperand> &imm8m,
           const CondKind cond = kAL)
        : Inst(kInsOrr, cond)
        , Rd(std::move(Rd))
        , Rn(std::move(Rn))
        , Rm_imm(imm8m) {
        CheckImm();
    }

    std::string Str() const override;

  private:
    const std::shared_ptr<RegOperand> Rd;
    const std::shared_ptr<RegOperand> Rn;
    const std::shared_ptr<Operand> Rm_imm;

    void CheckImm() const;
};

// nop{cond}    @ pseudo-instruction
class InsNop : public Inst {
  public:
    explicit InsNop(const CondKind cond = kAL) : Inst(kInsNop, cond) {}

    std::string Str() const override { return op_map[op] + cond_map[cond]; }
};

// label:
class InsLabel : public Inst {
  public:
    explicit InsLabel(std::shared_ptr<LabelOperand> label)
        : Inst(kInsLabel, kAL), label(std::move(label)) {}

    std::string Str() const override { return label->Str() + ':'; }

  private:
    const std::shared_ptr<LabelOperand> label;
};

class GlobalVar {
  public:
    GlobalVar(std::string name, std::vector<std::int32_t> init_value)
        : name(std::move(name)), init_value(std::move(init_value)) {}

    const std::string &GetName() const { return name; }

    void Dump(std::ostream &os) const;

  private:
    const std::string name;
    const std::vector<std::int32_t> init_value;
};

class Function {
  public:
    explicit Function(std::string name, const int argc = 0)
        : name(std::move(name)), argc(argc) {}

    const std::string &GetName() const { return name; }

    int GetArgc() const { return argc; }

    void AddInst(Inst *inst) { inst_list.emplace_back(inst); }
    void AddInst(std::shared_ptr<Inst> inst) { inst_list.emplace_back(inst); }
    const std::list<std::shared_ptr<Inst>> &GetInstList() const {
        return inst_list;
    }

    void Dump(std::ostream &os) const;

  private:
    const std::string name;
    const int argc;

    std::list<std::shared_ptr<Inst>> inst_list;
};

class Assembly {
  public:
    Assembly() = default;

    void AddVar(GlobalVar *var) {
        var_table.emplace(var->GetName(), var);
        var_list.emplace_back(var);
    }
    void AddVar(std::shared_ptr<GlobalVar> var) {
        var_table.emplace(var->GetName(), var);
        var_list.emplace_back(var);
    }

    void AddFunc(std::shared_ptr<Function> func) {
        func_table.emplace(func->GetName(), func);
        func_list.emplace_back(func);
    }
    void AddFunc(Function *func) {
        func_table.emplace(func->GetName(), func);
        func_list.emplace_back(func);
    }

    void Dump(std::ostream &os) const;

  private:
    std::unordered_map<std::string, std::shared_ptr<GlobalVar>> var_table;
    std::unordered_map<std::string, std::shared_ptr<Function>> func_table;
    // keep order
    std::vector<std::shared_ptr<GlobalVar>> var_list;
    std::vector<std::shared_ptr<Function>> func_list;
};

}  // namespace backend

#endif
