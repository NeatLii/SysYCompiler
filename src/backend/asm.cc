#include "backend/asm.h"

#include <memory>
#include <string>
#include <vector>

#include "backend/backend.h"
#include "backend/instruction.h"
#include "backend/operand.h"
#include "frontend/frontend.h"
#include "ir/ir.h"
#include "ir/type.h"
#include "ir/value.h"

backend::Assembly assembly;

int Assembling() {
    for (const auto &var : module->GetVarList()) {
        backend::TranslateGlobalVar(var);
    }
    for (const auto &func : module->GetFuncDefList()) {
        backend::TranslateFunction(func);
    }
    return 0;
}

namespace backend {

std::shared_ptr<RegOperand> RegPool::Get(const int id) {
    if (reg_pool.size() <= id) {
        for (auto i = reg_pool.size(); i < id + 1; ++i) {
            reg_pool.emplace_back(new RegOperand(static_cast<int>(i)));
        }
    }
    return reg_pool[id];
}

std::shared_ptr<RegOperand> RegPool::operator[](const int id) {
    if (reg_pool.size() <= id) {
        for (auto i = reg_pool.size(); i < id + 1; ++i) {
            reg_pool.emplace_back(new RegOperand(static_cast<int>(i)));
        }
    }
    return reg_pool[id];
}

void TranslateGlobalVar(const std::shared_ptr<ir::GlobalVarDef> &var_def) {
    auto name = var_def->GetName();
    std::vector<std::int32_t> init_value;

    if (var_def->IsZeroInit()) {
        int size = 1;
        for (int dim : var_def->GetIdent()
                           .GetType()
                           .Cast<ir::ArrayType>()
                           .GetArrDimList()) {
            size *= dim;
        }
        init_value = std::vector<std::int32_t>(size, 0);
    } else {
        for (const auto &imm : var_def->GetInitList()) {
            init_value.emplace_back(imm->GetValue());
        }
    }

    assembly.AddVar(std::make_shared<GlobalVar>(name, init_value));
}

static RegPool reg_pool;

void TranslateFunction(const std::shared_ptr<ir::FuncDef> &func_def) {
    auto func = std::make_shared<Function>(func_def->GetName());

    func->AddInst(new InsPush);
    func->AddInst(
        new InsMov(reg_pool[RegOperand::kFp], reg_pool[RegOperand::kSp]));
    func->AddInst(new InsSub(reg_pool[RegOperand::kFp],
                             reg_pool[RegOperand::kFp],
                             std::make_shared<ImmOperand>(4)));

    for (const auto &bb : func_def->GetBlockList()) {
        TranslateBasicBlock(func, bb);
    }
    assembly.AddFunc(func);
}

void TranslateBasicBlock(const std::shared_ptr<Function> &func,
                         const std::shared_ptr<ir::BasicBlock> &bb) {
    func->AddInst(
        new InsLabel('.' + func->GetName() + '_' + bb->GetLabel().GetName()));
    // TODO(neatlii): if(=-!!!a), int b = !a
    // bool check = false;
    // for (auto iter = bb->GetInstList().cbegin();
    //      iter != bb->GetInstList().cend(); ++iter) {
    //     if ((*iter)->kind == ir::Inst::kIcmp) {
    //         check = true;
    //         continue;
    //     }
    //     if (check) {
    //         if ((*iter)->kind != ir::Inst::kBr) throw "error";
    //         check = false;
    //     }
    // }
    for (const auto &inst : bb->GetInstList()) TranslateInst(func, *inst);
}

void TranslateInst(const std::shared_ptr<Function> &func, ir::Inst &inst) {
    switch (inst.kind) {
        case ir::Inst::kRet:
            TranslateRetInst(func, inst.Cast<ir::RetInst>());
            break;
        case ir::Inst::kBr:
            TranslateBrInst(func, inst.Cast<ir::BrInst>());
            break;
        case ir::Inst::kBinaryOp:
            TranslateBinaryOpInst(func, inst.Cast<ir::BinaryOpInst>());
            break;
        case ir::Inst::kAlloca:
            TranslateAllocaInst(func, inst.Cast<ir::AllocaInst>());
            break;
        case ir::Inst::kLoad:
            TranslateLoadInst(func, inst.Cast<ir::LoadInst>());
            break;
        case ir::Inst::kStore:
            TranslateStoreInst(func, inst.Cast<ir::StoreInst>());
            break;
        case ir::Inst::kGetelementptr:
            TranslateGetelementptrInst(func,
                                       inst.Cast<ir::GetelementptrInst>());
            break;
        case ir::Inst::kBitcast:
            TranslateBitcastInst(func, inst.Cast<ir::BitcastInst>());
            break;
        default:
            return;
    }
}

void TranslateRetInst(const std::shared_ptr<Function> &func,
                      const ir::RetInst &inst) {
    // recover stack
    auto stack_size
        = std::make_shared<ImmOperand>(func->stack_state.size() * 4);
    if (stack_size->IsImm8m()) {
        func->AddInst(new InsAdd(reg_pool[RegOperand::kSp],
                                 reg_pool[RegOperand::kSp], stack_size));
    } else {
        func->AddInst(new InsLdr(reg_pool[4], stack_size));
        func->AddInst(new InsAdd(reg_pool[RegOperand::kSp],
                                 reg_pool[RegOperand::kSp], reg_pool[4]));
    }
    func->AddInst(new InsPop);

    // store ret value to r0
    if (inst.HasRet()) {
        const auto &ret = inst.GetRet();
        if (ret.kind == ir::Value::kImm) {
            auto ret_value = ret.Cast<ir::Imm>().GetValue();
            auto ret_imm = std::make_shared<ImmOperand>(ret_value);
            if (ret_imm->IsImm16() || ret_imm->IsImm8m()) {
                func->AddInst(new InsMov(reg_pool[0], ret_imm));
            } else {
                func->AddInst(new InsLdr(reg_pool[0], ret_imm));
            }
        } else {
            auto ret_name = ret.Cast<ir::Var>().GetName();
            auto ret_state = func->var_state.find(ret_name);
            if (ret_state->second < 0) {
                unsigned int pos = func->stack_state.size()
                                   - func->stack_state.find(ret_name)->second;
                func->AddInst(
                    new InsLdr(reg_pool[0], reg_pool[RegOperand::kSp],
                               std::make_shared<ImmOperand>(pos * 4)));
            } else {
                func->AddInst(
                    new InsMov(reg_pool[0], reg_pool[ret_state->second]));
            }
        }
    }

    // return
    func->AddInst(new InsBx);
}

static Inst::CondKind cond;

void TranslateBrInst(const std::shared_ptr<Function> &func,
                     const ir::BrInst &inst) {
    if (inst.HasDest()) {
        func->AddInst(new InsB(std::make_shared<LabelOperand>(
            '.' + func->GetName() + '_' + inst.GetDest().GetName())));
    } else {
        func->AddInst(
            new InsB(std::make_shared<LabelOperand>('.' + func->GetName() + '_'
                                                    + inst.GetTrue().GetName()),
                     cond));
        func->AddInst(new InsB(std::make_shared<LabelOperand>(
            '.' + func->GetName() + '_' + inst.GetFalse().GetName())));
    }
}

void TranslateBinaryOpInst(const std::shared_ptr<Function> &func,
                           const ir::BinaryOpInst &inst) {
    // store r1 in stack
    func->GetReg(1);
    auto result_name = inst.GetResult().GetName();
    func->reg_state[1] = result_name;
    func->var_state[result_name] = 1;

    std::shared_ptr<RegOperand> lhs;
    std::shared_ptr<RegOperand> rhs;
    std::shared_ptr<ImmOperand> imm;
    bool is_reverse = false;

    // op Rd, Rn, Rm
    // op Rd, Rn, imm
    // imm can only be third operator
    if (inst.GetLHS().kind == ir::Value::kImm) {
        is_reverse = true;
        // find rhs, or load rhs in r2
        auto rhs_name = inst.GetRHS().Cast<ir::Var>().GetName();
        if (func->var_state[rhs_name] < 0) {
            auto offset = std::make_shared<ImmOperand>(
                (func->stack_state.size()
                 - func->stack_state.find(rhs_name)->second)
                * 4);
            func->AddInst(
                new InsLdr(reg_pool[2], reg_pool[RegOperand::kSp], offset));
            lhs = reg_pool[2];
        } else {
            lhs = reg_pool[func->var_state[rhs_name]];
        }
        // get imm
        imm = std::make_shared<ImmOperand>(
            inst.GetLHS().Cast<ir::Imm>().GetValue());
    } else {
        // find lhs, or load lhs in r2
        auto lhs_name = inst.GetLHS().Cast<ir::Var>().GetName();
        if (func->var_state[lhs_name] < 0) {
            auto offset = std::make_shared<ImmOperand>(
                (func->stack_state.size()
                 - func->stack_state.find(lhs_name)->second)
                * 4);
            func->AddInst(
                new InsLdr(reg_pool[2], reg_pool[RegOperand::kSp], offset));
            lhs = reg_pool[2];
        } else {
            lhs = reg_pool[func->var_state[lhs_name]];
        }
        // find rhs, or load lhs in r3
        if (inst.GetRHS().kind == ir::Value::kImm) {
            imm = std::make_shared<ImmOperand>(
                inst.GetLHS().Cast<ir::Imm>().GetValue());
        } else {
            auto rhs_name = inst.GetRHS().Cast<ir::Var>().GetName();
            if (func->var_state[rhs_name] < 0) {
                auto offset = std::make_shared<ImmOperand>(
                    (func->stack_state.size()
                     - func->stack_state.find(rhs_name)->second)
                    * 4);
                func->AddInst(
                    new InsLdr(reg_pool[3], reg_pool[RegOperand::kSp], offset));
                rhs = reg_pool[3];
            } else {
                rhs = reg_pool[func->var_state[rhs_name]];
            }
        }
    }

    if (imm != nullptr && !imm->IsImm8m() && !imm->IsImm16()) {
        rhs = reg_pool[3];
        func->AddInst(new InsLdr(rhs, imm));
    }

    switch (inst.op_code) {
        case ir::BinaryOpInst::kAdd:
            if (rhs == nullptr) {
                func->AddInst(new InsAdd(reg_pool[1], lhs, imm));
            } else {
                func->AddInst(new InsAdd(reg_pool[1], lhs, rhs));
            }
            break;
        case ir::BinaryOpInst::kSub:
            if (rhs == nullptr) {
                if (is_reverse) {
                    func->AddInst(new InsRsb(reg_pool[1], lhs, imm));
                } else {
                    func->AddInst(new InsSub(reg_pool[1], lhs, imm));
                }
            } else {
                if (is_reverse) {
                    func->AddInst(new InsRsb(reg_pool[1], lhs, rhs));
                } else {
                    func->AddInst(new InsSub(reg_pool[1], lhs, rhs));
                }
            }
            break;
        case ir::BinaryOpInst::kMul:
            if (rhs == nullptr) {
                rhs = reg_pool[3];
                func->AddInst(new InsMov(rhs, imm));
            }
            func->AddInst(new InsAdd(reg_pool[1], lhs, rhs));
            break;
        case ir::BinaryOpInst::kSDiv:
            if (rhs == nullptr) {
                rhs = reg_pool[3];
                func->AddInst(new InsMov(rhs, imm));
            }
            func->AddInst(new InsSDiv(reg_pool[1], lhs, rhs));
            break;
        case ir::BinaryOpInst::kSRem:
            if (rhs == nullptr) {
                rhs = reg_pool[3];
                func->AddInst(new InsMov(rhs, imm));
            }
            func->AddInst(new InsSDiv(reg_pool[4], lhs, rhs));
            func->AddInst(new InsMul(reg_pool[4], rhs, reg_pool[4]));
            func->AddInst(new InsSub(reg_pool[1], lhs, reg_pool[4]));
            break;
    }
}

void TranslateAllocaInst(const std::shared_ptr<Function> &func,
                         const ir::AllocaInst &inst) {
    auto ptr_name = inst.GetResult().GetName();
    const auto &type
        = inst.GetResult().GetType().Cast<ir::PtrType>().GetPointee();

    // alloc int
    if (type.kind == ir::Type::kInt) {
        func->stack_state[ptr_name] = func->stack_state.size() + 1;  // stuff
        func->ptr_state[ptr_name] = func->stack_state.size();
        func->AddInst(new InsSub(reg_pool[RegOperand::kSp],
                                 reg_pool[RegOperand::kSp],
                                 std::make_shared<ImmOperand>(4)));
        return;
    }

    // alloc int[]
    int size = 1;
    for (int dim : type.Cast<ir::ArrayType>().GetArrDimList()) size *= dim;
    func->ptr_state[ptr_name] = func->stack_state.size() + 1;
    for (int i = 0; i < size; ++i) {  // stuffs
        func->stack_state[ptr_name + '_' + std::to_string(i)]
            = func->stack_state.size() + 1;
    }
    auto offset = std::make_shared<ImmOperand>(size * 4);
    if (offset->IsImm8m() || offset->IsImm16()) {
        func->AddInst(new InsSub(reg_pool[RegOperand::kSp],
                                 reg_pool[RegOperand::kSp], offset));
    } else {
        func->AddInst(new InsLdr(reg_pool[4], offset));
        func->AddInst(new InsSub(reg_pool[RegOperand::kSp],
                                 reg_pool[RegOperand::kSp], reg_pool[4]));
    }
}

void TranslateLoadInst(const std::shared_ptr<Function> &func,
                       const ir::LoadInst &inst) {
    // ldr to r6 or r7
    std::shared_ptr<RegOperand> des_reg;
    auto des_name = inst.GetResult().GetName();
    if (func->reg_state[6].empty()) {
        des_reg = reg_pool[6];
        func->reg_state[6] = des_name;
        func->var_state[des_name] = 6;
    } else if (func->reg_state[7].empty()) {
        des_reg = reg_pool[7];
        func->reg_state[7] = des_name;
        func->var_state[des_name] = 7;
    } else {
        func->GetReg(6);
        // ldr to r6
        des_reg = reg_pool[6];
        func->reg_state[6] = des_name;
        func->var_state[des_name] = 6;
    }

    auto ptr_name = inst.GetPtr().GetName();
    if (inst.GetPtr().kind == ir::Value::kGlobalVar) {
        auto label = std::make_shared<LabelOperand>(ptr_name);
        func->AddInst(new InsLdr(reg_pool[5], label));
        func->AddInst(new InsLdr(des_reg, reg_pool[5]));
    } else {
        auto offset = (func->stack_state.size()
                       - func->ptr_state.find(ptr_name)->second)
                      * 4;
        func->AddInst(new InsLdr(des_reg, reg_pool[RegOperand::kSp],
                                 std::make_shared<ImmOperand>(offset)));
    }
}

void TranslateStoreInst(const std::shared_ptr<Function> &func,
                        const ir::StoreInst &inst) {
    std::shared_ptr<RegOperand> value_reg;

    const auto &value = inst.GetValue();
    if (value.kind == ir::Value::kImm) {
        value_reg = reg_pool[8];
        func->AddInst(new InsLdr(
            value_reg,
            std::make_shared<ImmOperand>(value.Cast<ir::Imm>().GetValue())));
    } else {
        value_reg = reg_pool[func->var_state[value.Cast<ir::Var>().GetName()]];
    }

    auto ptr_name = inst.GetPtr().Cast<ir::Var>().GetName();
    if (inst.GetPtr().kind == ir::Value::kGlobalVar) {
        auto label = std::make_shared<LabelOperand>(ptr_name);
        func->AddInst(new InsLdr(reg_pool[5], label));
        func->AddInst(new InsStr(value_reg, reg_pool[5]));
    } else {
        auto offset = (func->stack_state.size()
                       - func->ptr_state.find(ptr_name)->second)
                      * 4;
        func->AddInst(new InsStr(value_reg, reg_pool[RegOperand::kSp],
                                 std::make_shared<ImmOperand>(offset)));
    }
}

void TranslateGetelementptrInst(const std::shared_ptr<Function> &func,
                                const ir::GetelementptrInst &inst) {
    // for (auto pair : func->ptr_state) {
    //     std::cout << pair.first << ' ' << pair.second << std::endl;
    // }

    unsigned int offset = 0;
    const auto &type = inst.GetPtr().GetType().Cast<ir::PtrType>().GetPointee();

    if (type.kind == ir::Type::kInt) {
        offset = inst.GetIdxList().front()->Cast<ir::Imm>().GetValue();
    } else {
        auto arr_list = type.Cast<ir::ArrayType>().GetArrDimList();
        auto idx_list = inst.GetIdxList();
        for (int i = 0; i < arr_list.size(); ++i) {
            offset += arr_list[i] * idx_list[i]->Cast<ir::Imm>().GetValue();
        }
        offset += idx_list.back()->Cast<ir::Imm>().GetValue();
    }

    auto result_name = inst.GetResult().GetName();
    auto ptr_name = inst.GetPtr().GetName();

    auto pos = func->ptr_state[ptr_name];
    func->ptr_state[result_name] = pos + offset;
}

void TranslateBitcastInst(const std::shared_ptr<Function> &func,
                          const ir::BitcastInst &inst) {
    auto old_name = inst.GetValue().GetName();
    auto new_name = inst.GetResult().GetName();

    auto iter = func->ptr_state.find(old_name);
    func->ptr_state[new_name] = iter->second;
}

}  // namespace backend
