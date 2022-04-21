#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "frontend/ast_manager.h"
#include "frontend/frontend.h"
#include "ir/ir.h"
#include "ir/type.h"
#include "ir/value.h"
#include "util.h"

static auto module = std::make_shared<ir::Module>();
static std::map<ast::ASTLocation, std::shared_ptr<ir::Var>> node_map;
static std::map<std::shared_ptr<ir::Var>, ast::ASTLocation> param_map;

// decl node to ir
void TranslateGlobalVarDecl(const ast::VarDecl &decl);
void TranslateLocalVarDecl(const std::shared_ptr<ir::BasicBlock> &bb,
                           const ast::VarDecl &decl,
                           int &tmp_id);
void TranslateFunctionDecl(const ast::FunctionDecl &decl);

// stmt node to ir
void TranslateStmt(const std::shared_ptr<ir::FuncDef> &def,
                   const std::shared_ptr<ir::BasicBlock> &bb,
                   const ast::Stmt &stmt,
                   int &tmp_id);
void TranslatCompoundStmt(const std::shared_ptr<ir::FuncDef> &def,
                          const ast::CompoundStmt &stmt,
                          int &tmp_id,
                          bool is_entry);
void TranslateDeclStmt(const std::shared_ptr<ir::FuncDef> &def,
                       const std::shared_ptr<ir::BasicBlock> &bb,
                       const ast::DeclStmt &stmt,
                       int &tmp_id);
void TranslateReturnStmt(const std::shared_ptr<ir::FuncDef> &def,
                         const std::shared_ptr<ir::BasicBlock> &bb,
                         const ast::ReturnStmt &stmt,
                         int &tmp_id);

// expr node to ir
std::shared_ptr<ir::Value> TranslateExpr(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::Expr &expr,
    int &tmp_id);
std::shared_ptr<ir::Value> TranslateDeclRefExpr(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::DeclRefExpr &expr,
    int &tmp_id);
std::shared_ptr<ir::Value> TranslateCallExpr(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::CallExpr &expr,
    int &tmp_id,
    bool has_ret);
std::shared_ptr<ir::Value> TranslateBinaryOperator(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::BinaryOperator &expr,
    int &tmp_id);
void TranslateAssignOperator(const std::shared_ptr<ir::BasicBlock> &bb,
                             const ast::BinaryOperator &expr,
                             int &tmp_id);
std::shared_ptr<ir::Value> TranslatekUnaryOperator(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::UnaryOperator &expr,
    int &tmp_id);

/* definitions */

std::shared_ptr<ir::Module> AstToIR() {
    ast::TranslationUnit &root = ast_manager.GetRoot();
    for (auto decl_loc : root.GetDeclList()) {
        auto &decl = ast_manager.GetDecl(decl_loc);
        if (decl.kind == ast::ASTNode::kVarDecl) {
            TranslateGlobalVarDecl(decl.Cast<ast::VarDecl>());
        } else {
            TranslateFunctionDecl(decl.Cast<ast::FunctionDecl>());
        }
    }
    return module;
}

ir::Type *GetVarType(const ast::VarDecl &decl) {
    if (decl.IsArray()) {
        std::vector<int> arr_dim_list;
        for (auto expr : decl.GetArrDimList()) {
            arr_dim_list.emplace_back(ast_manager.GetExpr(expr).GetValue());
        }
        return new ir::ArrayType(arr_dim_list);
    }
    return new ir::IntType(ir::IntType::kI32);
}

void TranslateGlobalVarDecl(const ast::VarDecl &decl) {
    auto global_var = std::make_shared<ir::GlobalVar>(GetVarType(decl),
                                                      decl.GetIdentName());
    auto global_var_ptr = std::make_shared<ir::GlobalVar>(
        new ir::PtrType(GetVarType(decl)), decl.GetIdentName());
    node_map.emplace(decl.GetLocation(), global_var_ptr);

    std::vector<std::shared_ptr<ir::Imm>> init_list;
    if (decl.IsArray()) {
        if (!decl.HasInit() || decl.GetInitList().GetInitList().empty()) {
            module->AddVar(new ir::GlobalVarDef(global_var, decl.IsConst(),
                                                init_list, true));
        } else {
            for (auto init_val : decl.GetInitList().GetInitMap()) {
                init_list.emplace_back(new ir::Imm(init_val));
            }
            module->AddVar(new ir::GlobalVarDef(global_var, decl.IsConst(),
                                                init_list, false));
        }
    } else {
        int init_val = decl.HasInit() ? decl.GetInit().GetValue() : 0;
        init_list.emplace_back(new ir::Imm(init_val));
        module->AddVar(
            new ir::GlobalVarDef(global_var, decl.IsConst(), init_list, false));
    }
}

void TranslateLocalVarDecl(const std::shared_ptr<ir::BasicBlock> &bb,
                           const ast::VarDecl &decl,
                           int &tmp_id) {
    auto local_var_ptr = std::make_shared<ir::TmpVar>(
        new ir::PtrType(GetVarType(decl)), tmp_id++);
    node_map.emplace(decl.GetLocation(), local_var_ptr);

    bb->AddInst(new ir::AllocaInst(local_var_ptr));

    if (decl.HasInit()) {
        if (decl.IsArray()) {
            auto list = decl.GetInitList().GetInitMapExpr();
            auto new_local_var_ptr
                = std::make_shared<ir::TmpVar>(new ir::PtrType, tmp_id++);
            bb->AddInst(new ir::BitcastInst(new_local_var_ptr, local_var_ptr));
            int offset = 0;
            for (auto value : list) {
                std::shared_ptr<ir::Value> init_val;
                if (value.first) {
                    const auto &expr = ast_manager.GetExpr(value.second);
                    if (expr.IsConst()) {
                        init_val.reset(new ir::Imm(expr.GetValue()));
                    } else {
                        init_val = TranslateExpr(bb, expr, tmp_id);
                    }
                } else {
                    init_val.reset(new ir::Imm(0));
                }
                auto addr
                    = std::make_shared<ir::TmpVar>(new ir::PtrType, tmp_id++);
                auto idx = std::make_shared<ir::Imm>(offset++);
                bb->AddInst(
                    new ir::GetelementptrInst(addr, new_local_var_ptr, {idx}));
                bb->AddInst(new ir::StoreInst(init_val, addr));
            }
        } else {
            std::shared_ptr<ir::Value> init_val;
            if (decl.GetInit().IsConst()) {
                init_val.reset(new ir::Imm(decl.GetInit().GetValue()));
            } else {
                init_val = TranslateExpr(bb, decl.GetInit(), tmp_id);
            }
            bb->AddInst(new ir::StoreInst(init_val, local_var_ptr));
        }
    }
}

void TranslateFunctionDecl(const ast::FunctionDecl &decl) {
    ir::Type *ret_type = nullptr;
    if (decl.GetType() == ast::Decl::kVoid) {
        ret_type = new ir::VoidType;
    } else {
        ret_type = new ir::IntType(ir::IntType::kI32);
    }

    int tmp_id = 0;
    std::vector<std::shared_ptr<ir::Type>> param_type_list;
    std::vector<std::shared_ptr<ir::TmpVar>> param_list;
    for (auto param_loc : decl.GetParamList()) {
        const auto &param
            = ast_manager.GetNode(param_loc).Cast<ast::ParamVarDecl>();
        std::shared_ptr<ir::Type> type;

        if (param.IsArrayPtr()) {
            std::vector<int> arr_dim_list;
            for (auto expr : param.GetArrDimList()) {
                arr_dim_list.emplace_back(ast_manager.GetExpr(expr).GetValue());
            }
            type = std::make_shared<ir::ArrayType>(arr_dim_list);
        } else if (param.IsPtr()) {
            type = std::make_shared<ir::PtrType>(
                new ir::IntType(ir::IntType::kI32));
        } else {
            type = std::make_shared<ir::IntType>(ir::IntType::kI32);
        }
        param_type_list.emplace_back(type);
        auto tmp_var = std::make_shared<ir::TmpVar>(type, tmp_id++);
        param_map.emplace(tmp_var, param.GetLocation());
        param_list.emplace_back(tmp_var);
    }

    auto func = std::make_shared<ir::GlobalVar>(
        new ir::FuncType(ret_type, param_type_list), decl.GetIdentName());
    node_map.emplace(decl.GetLocation(), func);

    if (decl.HasDef()) {
        auto func_def = std::make_shared<ir::FuncDef>(func, param_list);
        module->AddFuncDef(func_def);
        TranslatCompoundStmt(func_def, decl.GetDef(), tmp_id, true);
    } else {
        module->AddFuncDecl(new ir::FuncDecl(func));
    }
}

void TranslateStmt(const std::shared_ptr<ir::FuncDef> &def,
                   const std::shared_ptr<ir::BasicBlock> &bb,
                   const ast::Stmt &stmt,
                   int &tmp_id) {
    switch (stmt.kind) {
        case ast::ASTNode::kCompoundStmt:
            // TODO(neatlii):
            break;
        case ast::ASTNode::kDeclStmt:
            TranslateDeclStmt(def, bb, stmt.Cast<ast::DeclStmt>(), tmp_id);
            break;
        case ast::ASTNode::kNullStmt:
            break;
        case ast::ASTNode::kIfStmt:
            break;
        case ast::ASTNode::kWhileStmt:
            break;
        case ast::ASTNode::kContinueStmt:
            break;
        case ast::ASTNode::kBreakStmt:
            break;
        case ast::ASTNode::kReturnStmt:
            TranslateReturnStmt(def, bb, stmt.Cast<ast::ReturnStmt>(), tmp_id);
            break;
        case ast::ASTNode::kBinaryOperator:
            if (stmt.Cast<ast::BinaryOperator>().op_code
                == ast::BinaryOperator::kAssign) {
                TranslateAssignOperator(bb, stmt.Cast<ast::BinaryOperator>(),
                                        tmp_id);
            }
            break;
        case ast::ASTNode::kCallExpr:
            TranslateCallExpr(bb, stmt.Cast<ast::CallExpr>(), tmp_id, false);
            break;
        default:
            break;
    }
}

void TranslatCompoundStmt(const std::shared_ptr<ir::FuncDef> &def,
                          const ast::CompoundStmt &stmt,
                          int &tmp_id,
                          const bool is_entry) {
    const std::string bb_name = is_entry ? "entry" : std::to_string(tmp_id++);
    auto bb = std::make_shared<ir::BasicBlock>(
        new ir::LocalVar(new ir::LabelType, bb_name));

    if (is_entry) {
        for (const auto &param : def->GetParamList()) {
            auto param_ptr = std::make_shared<ir::TmpVar>(
                new ir::PtrType(param->GetTypePtr()), tmp_id++);
            bb->AddInst(new ir::AllocaInst(param_ptr));
            bb->AddInst(new ir::StoreInst(param, param_ptr));
            node_map.emplace(param_map.find(param)->second, param_ptr);
        }
    }

    for (auto stmt_loc : stmt.GetStmtList()) {
        const auto &stmt = ast_manager.GetStmt(stmt_loc);
        TranslateStmt(def, bb, stmt, tmp_id);
    }
    def->AddBlock(bb);
}

void TranslateDeclStmt(const std::shared_ptr<ir::FuncDef> &def,
                       const std::shared_ptr<ir::BasicBlock> &bb,
                       const ast::DeclStmt &stmt,
                       int &tmp_id) {
    for (auto decl_loc : stmt.GetDeclList()) {
        const auto &decl = ast_manager.GetNode(decl_loc).Cast<ast::VarDecl>();
        TranslateLocalVarDecl(bb, decl, tmp_id);
    }
}

void TranslateReturnStmt(const std::shared_ptr<ir::FuncDef> &def,
                         const std::shared_ptr<ir::BasicBlock> &bb,
                         const ast::ReturnStmt &stmt,
                         int &tmp_id) {
    if (stmt.HasExpr()) {
        std::shared_ptr<ir::Value> ret_expr;
        if (stmt.GetExpr().IsConst()) {
            ret_expr.reset(new ir::Imm(stmt.GetExpr().GetValue()));
        } else {
            ret_expr = TranslateExpr(bb, stmt.GetExpr(), tmp_id);
        }
        bb->AddInst(new ir::RetInst(ret_expr));
    } else {
        bb->AddInst(new ir::RetInst);
    }
}

std::shared_ptr<ir::Value> TranslateExpr(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::Expr &expr,
    int &tmp_id) {
    switch (expr.kind) {
        case ast::ASTNode::kParenExpr:
            return TranslateExpr(bb, expr.Cast<ast::ParenExpr>().GetSubExpr(),
                                 tmp_id);
        case ast::ASTNode::kDeclRefExpr:
            return TranslateDeclRefExpr(bb, expr.Cast<ast::DeclRefExpr>(),
                                        tmp_id);
        case ast::ASTNode::kCallExpr:
            return TranslateCallExpr(bb, expr.Cast<ast::CallExpr>(), tmp_id,
                                     true);
        case ast::ASTNode::kBinaryOperator:
            return TranslateBinaryOperator(bb, expr.Cast<ast::BinaryOperator>(),
                                           tmp_id);
        case ast::ASTNode::kUnaryOperator:
            return TranslatekUnaryOperator(bb, expr.Cast<ast::UnaryOperator>(),
                                           tmp_id);
        default:
            break;
    }
    return nullptr;
}

std::shared_ptr<ir::Value> TranslateDeclRefExpr(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::DeclRefExpr &expr,
    int &tmp_id) {
    auto ptr = node_map.find(expr.GetRef().GetLocation())->second;

    if (expr.IsConst()) return std::make_shared<ir::Imm>(expr.GetValue());
    if (expr.IsArray()) {
        auto addr = std::make_shared<ir::TmpVar>(new ir::PtrType, tmp_id++);
        std::vector<std::shared_ptr<ir::Value>> idx_list;
        idx_list.emplace_back(new ir::Imm(0));
        for (auto expr_loc : expr.GetArrDimList()) {
            const auto &expr = ast_manager.GetExpr(expr_loc);
            if (expr.IsConst()) {
                idx_list.emplace_back(new ir::Imm(expr.GetValue()));
            } else {
                idx_list.emplace_back(TranslateExpr(bb, expr, tmp_id));
            }
        }
        bb->AddInst(new ir::GetelementptrInst(addr, ptr, idx_list));
        auto result = std::make_shared<ir::TmpVar>(tmp_id++);
        bb->AddInst(new ir::LoadInst(result, addr));
        return result;
    }
    auto result = std::make_shared<ir::TmpVar>(tmp_id++);
    bb->AddInst(new ir::LoadInst(result, ptr));
    return result;
}

std::shared_ptr<ir::Value> TranslateCallExpr(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::CallExpr &expr,
    int &tmp_id,
    const bool has_ret) {
    auto func = node_map.find(expr.GetRef().GetLocation())->second;
    std::vector<std::shared_ptr<ir::Value>> param_list;
    for (auto expr_loc : expr.GetParamList()) {
        const auto &expr = ast_manager.GetExpr(expr_loc);
        if (expr.IsConst()) {
            param_list.emplace_back(new ir::Imm(expr.GetValue()));
        } else {
            param_list.emplace_back(TranslateExpr(bb, expr, tmp_id));
        }
    }
    if (has_ret) {
        auto result = std::make_shared<ir::TmpVar>(tmp_id++);
        bb->AddInst(new ir::CallInst(result, func, param_list));
        return result;
    }
    bb->AddInst(new ir::CallInst(func, param_list));
    return nullptr;
}

std::shared_ptr<ir::Value> TranslateBinaryOperator(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::BinaryOperator &expr,
    int &tmp_id) {
    std::shared_ptr<ir::TmpVar> result;
    std::shared_ptr<ir::Value> lhs;
    std::shared_ptr<ir::Value> rhs;

    if (expr.GetLHS().IsConst()) {
        lhs.reset(new ir::Imm(expr.GetLHS().GetValue()));
    } else {
        lhs = TranslateExpr(bb, expr.GetLHS(), tmp_id);
    }
    if (expr.GetRHS().IsConst()) {
        rhs.reset(new ir::Imm(expr.GetRHS().GetValue()));
    } else {
        rhs = TranslateExpr(bb, expr.GetRHS(), tmp_id);
    }

    if (ast::BinaryOperator::kAdd <= expr.op_code
        && expr.op_code <= ast::BinaryOperator::kRem) {
        result = std::make_shared<ir::TmpVar>(
            new ir::IntType(ir::IntType::kI32), tmp_id++);
    } else {
        result = std::make_shared<ir::TmpVar>(new ir::IntType(ir::IntType::kI1),
                                              tmp_id++);
    }

    // TODO(neatlii): zext check.
    switch (expr.op_code) {
        case ast::BinaryOperator::kAdd:
            bb->AddInst(
                new ir::BinaryOpInst(ir::BinaryOpInst::kAdd, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kSub:
            bb->AddInst(
                new ir::BinaryOpInst(ir::BinaryOpInst::kSub, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kMul:
            bb->AddInst(
                new ir::BinaryOpInst(ir::BinaryOpInst::kMul, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kDiv:
            bb->AddInst(new ir::BinaryOpInst(ir::BinaryOpInst::kSDiv, result,
                                             lhs, rhs));
            break;
        case ast::BinaryOperator::kRem:
            bb->AddInst(new ir::BinaryOpInst(ir::BinaryOpInst::kSRem, result,
                                             lhs, rhs));
            break;
        case ast::BinaryOperator::kOr:
            bb->AddInst(new ir::BitwiseOpInst(ir::BitwiseOpInst::kOr, result,
                                              lhs, rhs));
            break;
        case ast::BinaryOperator::kAnd:
            bb->AddInst(new ir::BitwiseOpInst(ir::BitwiseOpInst::kAnd, result,
                                              lhs, rhs));
            break;
        case ast::BinaryOperator::kEQ:
            bb->AddInst(new ir::IcmpInst(ir::IcmpInst::kEQ, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kNE:
            bb->AddInst(new ir::IcmpInst(ir::IcmpInst::kNE, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kLT:
            bb->AddInst(new ir::IcmpInst(ir::IcmpInst::kSLT, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kLE:
            bb->AddInst(new ir::IcmpInst(ir::IcmpInst::kSLE, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kGT:
            bb->AddInst(new ir::IcmpInst(ir::IcmpInst::kSGT, result, lhs, rhs));
            break;
        case ast::BinaryOperator::kGE:
            bb->AddInst(new ir::IcmpInst(ir::IcmpInst::kSGE, result, lhs, rhs));
            break;
        default:
            break;
    }

    return result;
}

void TranslateAssignOperator(const std::shared_ptr<ir::BasicBlock> &bb,
                             const ast::BinaryOperator &expr,
                             int &tmp_id) {
    const auto &ref_expr = expr.GetLHS().Cast<ast::DeclRefExpr>();
    auto ptr = node_map.find(ref_expr.GetRef().GetLocation())->second;

    std::shared_ptr<ir::Var> addr;
    if (ref_expr.IsArray()) {
        addr = std::make_shared<ir::TmpVar>(new ir::PtrType(), tmp_id++);
        std::vector<std::shared_ptr<ir::Value>> idx_list;
        for (auto expr_loc : ref_expr.GetArrDimList()) {
            const auto &expr = ast_manager.GetExpr(expr_loc);
            if (expr.IsConst()) {
                idx_list.emplace_back(new ir::Imm(expr.GetValue()));
            } else {
                idx_list.emplace_back(TranslateExpr(bb, expr, tmp_id));
            }
        }
        bb->AddInst(new ir::GetelementptrInst(addr, ptr, idx_list));
    } else {
        addr = ptr;
    }

    std::shared_ptr<ir::Value> value;
    if (expr.GetRHS().IsConst()) {
        value = std::make_shared<ir::Imm>(expr.GetRHS().GetValue());
    } else {
        value = TranslateExpr(bb, expr.GetRHS(), tmp_id);
    }

    bb->AddInst(new ir::StoreInst(value, addr));
}

std::shared_ptr<ir::Value> TranslatekUnaryOperator(
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::UnaryOperator &expr,
    int &tmp_id) {
    std::shared_ptr<ir::TmpVar> result;
    std::shared_ptr<ir::Value> sub_expr;
    auto zero = std::make_shared<ir::Imm>(0);

    if (expr.GetSubExpr().IsConst()) {
        sub_expr.reset(new ir::Imm(expr.GetSubExpr().GetValue()));
    } else {
        sub_expr = TranslateExpr(bb, expr.GetSubExpr(), tmp_id);
    }

    if (ast::UnaryOperator::kPlus <= expr.op_code
        && expr.op_code <= ast::UnaryOperator::kMinus) {
        result = std::make_shared<ir::TmpVar>(
            new ir::IntType(ir::IntType::kI32), tmp_id++);
    } else {
        result = std::make_shared<ir::TmpVar>(new ir::IntType(ir::IntType::kI1),
                                              tmp_id++);
    }

    // TODO(neatlii): zext check.
    switch (expr.op_code) {
        case ast::UnaryOperator::kPlus:
            return sub_expr;
        case ast::UnaryOperator::kMinus:
            bb->AddInst(new ir::BinaryOpInst(ir::BinaryOpInst::kSub, result,
                                             zero, sub_expr));
            break;
        case ast::UnaryOperator::kNot:
            bb->AddInst(
                new ir::IcmpInst(ir::IcmpInst::kEQ, result, sub_expr, zero));
            break;
    }

    return result;
}
