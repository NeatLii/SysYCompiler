#include "frontend/ast_to_ir.h"

#include <iostream>
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "frontend/ast_manager.h"
#include "frontend/frontend.h"
#include "ir/ir.h"
#include "ir/type.h"
#include "ir/value.h"
#include "util.h"

std::shared_ptr<ir::Module> module = std::make_shared<ir::Module>();

static std::unordered_map<ast::ASTLocation, std::shared_ptr<ir::Var>> node_map;

static std::stack<std::list<std::shared_ptr<ir::BasicBlock>>> break_stack;
static std::stack<std::list<std::shared_ptr<ir::BasicBlock>>> continue_stack;

static std::stack<std::list<std::shared_ptr<ir::BasicBlock>>> true_stack;
static std::stack<std::list<std::shared_ptr<ir::BasicBlock>>> false_stack;

int AstToIR() {
    ast::TranslationUnit &root = ast_manager.GetRoot();
    for (auto decl_loc : root.GetDeclList()) {
        auto &decl = ast_manager.GetDecl(decl_loc);
        if (decl.kind == ast::ASTNode::kVarDecl) {
            frontend::TranslateGlobalVarDecl(decl.Cast<ast::VarDecl>());
        } else {
            frontend::TranslateFunctionDecl(decl.Cast<ast::FunctionDecl>());
        }
    }
    return 0;
}

namespace frontend {

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

    // TODO(neatlii): ; multiple dimension arr init
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

void TranslateLocalVarDecl(const std::shared_ptr<ir::FuncDef> &def,
                           const std::shared_ptr<ir::BasicBlock> &bb,
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
                        init_val = TranslateExpr(def, bb, expr, tmp_id);
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
                init_val = TranslateExpr(def, bb, decl.GetInit(), tmp_id);
            }
            bb->AddInst(new ir::StoreInst(init_val, local_var_ptr));
        }
    }
}

void TranslateParamVarDecl(const std::shared_ptr<ir::BasicBlock> &bb,
                           const ast::ParamVarDecl &decl,
                           int &tmp_id) {
    auto param = node_map.find(decl.GetLocation())->second;
    std::shared_ptr<ir::Type> local_type
        = std::make_shared<ir::PtrType>(param->GetTypePtr());

    auto param_local = std::make_shared<ir::TmpVar>(local_type, tmp_id++);
    bb->AddInst(new ir::AllocaInst(param_local));
    bb->AddInst(new ir::StoreInst(param, param_local));
    node_map[decl.GetLocation()] = param_local;
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
            type = std::make_shared<ir::PtrType>(
                new ir::ArrayType(arr_dim_list));
        } else if (param.IsPtr()) {
            type = std::make_shared<ir::PtrType>(
                new ir::IntType(ir::IntType::kI32));
        } else {
            type = std::make_shared<ir::IntType>(ir::IntType::kI32);
        }

        param_type_list.emplace_back(type);
        auto tmp_var = std::make_shared<ir::TmpVar>(type, tmp_id++);
        node_map.emplace(param_loc, tmp_var);
        param_list.emplace_back(tmp_var);
    }

    auto func = std::make_shared<ir::GlobalVar>(
        new ir::FuncType(ret_type, param_type_list), decl.GetIdentName());
    node_map.emplace(decl.GetLocation(), func);
    if (decl.HasDef()) {
        auto func_def = std::make_shared<ir::FuncDef>(func, param_list);
        module->AddFuncDef(func_def);
        auto bb = std::make_shared<ir::BasicBlock>(
            new ir::LocalVar(new ir::LabelType, "entry"));
        func_def->AddBlock(bb);
        for (auto param_loc : decl.GetParamList()) {
            TranslateParamVarDecl(
                bb, ast_manager.GetNode(param_loc).Cast<ast::ParamVarDecl>(),
                tmp_id);
        }
        auto bb_end = TranslatCompoundStmt(func_def, bb, decl.GetDef(), tmp_id);
        if (bb_end->GetInstList().empty()
            || !bb_end->GetInstList().back()->IsTerminateInst()) {
            if (decl.GetType() == ast::Decl::kVoid) {
                bb_end->AddInst(new ir::RetInst);
            } else {
                bb_end->AddInst(new ir::RetInst(new ir::Imm(0)));
            }
        }
    } else {
        module->AddFuncDecl(new ir::FuncDecl(func));
    }
}

std::shared_ptr<ir::BasicBlock> TranslateStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::Stmt &stmt,
    int &tmp_id) {
    switch (stmt.kind) {
        case ast::ASTNode::kCompoundStmt:
            return TranslatCompoundStmt(def, bb, stmt.Cast<ast::CompoundStmt>(),
                                        tmp_id);
        case ast::ASTNode::kDeclStmt:
            TranslateDeclStmt(def, bb, stmt.Cast<ast::DeclStmt>(), tmp_id);
            return bb;
        case ast::ASTNode::kIfStmt:
            return TranslateIfStmt(def, bb, stmt.Cast<ast::IfStmt>(), tmp_id);
        case ast::ASTNode::kWhileStmt:
            return TranslateWhileStmt(def, bb, stmt.Cast<ast::WhileStmt>(),
                                      tmp_id);
        case ast::ASTNode::kContinueStmt:
            bb->AddInst(new ir::RetInst);
            continue_stack.top().emplace_back(bb);
            return bb;
        case ast::ASTNode::kBreakStmt:
            bb->AddInst(new ir::RetInst);
            break_stack.top().emplace_back(bb);
            return bb;
        case ast::ASTNode::kReturnStmt:
            TranslateReturnStmt(def, bb, stmt.Cast<ast::ReturnStmt>(), tmp_id);
            return bb;
        case ast::ASTNode::kBinaryOperator:
            if (stmt.Cast<ast::BinaryOperator>().op_code
                == ast::BinaryOperator::kAssign) {
                TranslateAssignOperator(
                    def, bb, stmt.Cast<ast::BinaryOperator>(), tmp_id);
            }
            return bb;
        case ast::ASTNode::kCallExpr:
            TranslateCallExpr(def, bb, stmt.Cast<ast::CallExpr>(), tmp_id,
                              false);
            return bb;
        default:
            return bb;
    }
}

std::shared_ptr<ir::BasicBlock> TranslatCompoundStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::CompoundStmt &stmt,
    int &tmp_id) {
    auto result = bb;
    for (auto stmt_loc : stmt.GetStmtList()) {
        const auto &stmt = ast_manager.GetStmt(stmt_loc);
        auto next = TranslateStmt(def, result, stmt, tmp_id);
        if (next != nullptr) result = next;
    }
    return result;
}

void TranslateDeclStmt(const std::shared_ptr<ir::FuncDef> &def,
                       const std::shared_ptr<ir::BasicBlock> &bb,
                       const ast::DeclStmt &stmt,
                       int &tmp_id) {
    for (auto decl_loc : stmt.GetDeclList()) {
        const auto &decl = ast_manager.GetNode(decl_loc).Cast<ast::VarDecl>();
        TranslateLocalVarDecl(def, bb, decl, tmp_id);
    }
}

std::shared_ptr<ir::BasicBlock> TranslateIfStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::IfStmt &stmt,
    int &tmp_id) {
    auto zero_i32 = std::make_shared<ir::Imm>(0);

    const auto &cond_expr = stmt.GetCond();
    if (cond_expr.IsConst()) {
        if (cond_expr.GetValue() == 0 && !stmt.HasElse()) return bb;
        return TranslateStmt(
            def, bb,
            cond_expr.GetValue() == 0 ? stmt.GetElse() : stmt.GetThen(),
            tmp_id);
    }

    true_stack.emplace();
    false_stack.emplace();

    auto cond_var = TranslateExpr(def, bb, cond_expr, tmp_id);

    if (cond_var != nullptr
        && cond_var->GetType().Cast<ir::IntType>().GetWidth()
               == ir::IntType::kI32) {
        auto result = std::make_shared<ir::TmpVar>(
            new ir::IntType(ir::IntType::kI1), tmp_id++);
        bb->AddInst(
            new ir::IcmpInst(ir::IcmpInst::kNE, result, cond_var, zero_i32));
        cond_var = result;
    }

    // Then branch
    auto label_then = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    auto bb_then = std::make_shared<ir::BasicBlock>(label_then);
    def->AddBlock(bb_then);
    bb->AddSuccessor(bb_then);
    bb_then->AddPredecessor(bb);
    auto then_end = TranslateStmt(def, bb_then, stmt.GetThen(), tmp_id);

    // Else branch
    std::shared_ptr<ir::TmpVar> label_else;
    std::shared_ptr<ir::BasicBlock> bb_else;
    std::shared_ptr<ir::BasicBlock> else_end;
    if (stmt.HasElse()) {
        label_else = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
        bb_else = std::make_shared<ir::BasicBlock>(label_else);
        def->AddBlock(bb_else);
        else_end = TranslateStmt(def, bb_else, stmt.GetElse(), tmp_id);
    }

    // End branch
    auto label_end = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    auto bb_end = std::make_shared<ir::BasicBlock>(label_end);
    def->AddBlock(bb_end);
    if (then_end->GetInstList().empty()
        || then_end->GetInstList().back()->kind != ir::Inst::kRet) {
        then_end->AddInst(new ir::BrInst(label_end));
        then_end->AddSuccessor(bb_end);
        bb_end->AddPredecessor(then_end);
    }
    if (stmt.HasElse()) {
        if (else_end->GetInstList().empty()
            || else_end->GetInstList().back()->kind != ir::Inst::kRet) {
            else_end->AddInst(new ir::BrInst(label_end));
            else_end->AddSuccessor(bb_end);
            bb_end->AddPredecessor(else_end);
        }
        if (cond_var != nullptr) {
            bb->AddInst(new ir::BrInst(cond_var, label_then, label_else));
            bb->AddSuccessor(bb_else);
            bb_else->AddPredecessor(bb);
            true_stack.pop();
            false_stack.pop();
            return bb_end;
        }
    }
    if (cond_var != nullptr) {
        bb->AddInst(new ir::BrInst(cond_var, label_then, label_end));
        bb->AddSuccessor(bb_end);
        bb_end->AddPredecessor(bb);
    }

    for (const auto &bb_true : true_stack.top()) {
        bb_true->GetInstList().back()->Cast<ir::BrInst>().SetTrue(label_then);
        bb_true->AddSuccessor(bb_then);
        bb_then->AddPredecessor(bb_true);
    }
    true_stack.pop();

    for (const auto &bb_false : false_stack.top()) {
        if (stmt.HasElse()) {
            bb_false->GetInstList().back()->Cast<ir::BrInst>().SetFalse(
                label_else);
            bb_false->AddSuccessor(bb_else);
            bb_else->AddPredecessor(bb_false);
            continue;
        }
        bb_false->GetInstList().back()->Cast<ir::BrInst>().SetFalse(label_end);
        bb_false->AddSuccessor(bb_end);
        bb_end->AddPredecessor(bb_false);
    }
    false_stack.pop();

    return bb_end;
}

std::shared_ptr<ir::BasicBlock> TranslateWhileStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::WhileStmt &stmt,
    int &tmp_id) {
    const auto &cond_expr = stmt.GetCond();
    auto zero_i32 = std::make_shared<ir::Imm>(0);

    if (cond_expr.IsConst()) {
        if (cond_expr.GetValue() == 0) return bb;
    }

    break_stack.emplace();
    continue_stack.emplace();

    true_stack.emplace();
    false_stack.emplace();

    // Check branch
    auto label_check
        = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    auto bb_check = std::make_shared<ir::BasicBlock>(label_check);
    def->AddBlock(bb_check);
    bb->AddInst(new ir::BrInst(label_check));
    bb->AddSuccessor(bb_check);
    bb_check->AddPredecessor(bb);

    std::shared_ptr<ir::Value> cond_var;
    if (!cond_expr.IsConst()) {
        cond_var = TranslateExpr(def, bb_check, cond_expr, tmp_id);
        if (cond_var != nullptr
            && cond_var->GetType().Cast<ir::IntType>().GetWidth()
                   == ir::IntType::kI32) {
            auto result = std::make_shared<ir::TmpVar>(
                new ir::IntType(ir::IntType::kI1), tmp_id++);
            bb_check->AddInst(new ir::IcmpInst(ir::IcmpInst::kNE, result,
                                               cond_var, zero_i32));
            cond_var = result;
        }
    }

    // Body branch
    auto label_body = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    auto bb_body = std::make_shared<ir::BasicBlock>(label_body);
    def->AddBlock(bb_body);
    bb_check->AddSuccessor(bb_body);
    bb_body->AddPredecessor(bb_check);
    auto body_end = TranslateStmt(def, bb_body, stmt.GetBody(), tmp_id);
    if (body_end->GetInstList().empty()
        || body_end->GetInstList().back()->kind != ir::Inst::kRet) {
        body_end->AddInst(new ir::BrInst(label_check));
        bb_body->AddSuccessor(bb_check);
        bb_check->AddPredecessor(bb_body);
    }

    // End branch
    auto label_end = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    auto bb_end = std::make_shared<ir::BasicBlock>(label_end);
    def->AddBlock(bb_end);
    if (cond_expr.IsConst()) {
        bb_check->AddInst(new ir::BrInst(label_body));
    } else {
        if (cond_var != nullptr) {
            bb_check->AddInst(new ir::BrInst(cond_var, label_body, label_end));
        }
    }

    for (const auto &bb_break : break_stack.top()) {
        bb_break->GetInstList().pop_back();
        bb_break->AddInst(new ir::BrInst(label_end));
        bb_break->AddSuccessor(bb_end);
        bb_end->AddPredecessor(bb_break);
    }
    break_stack.pop();

    for (const auto &bb_continue : continue_stack.top()) {
        bb_continue->GetInstList().pop_back();
        bb_continue->AddInst(new ir::BrInst(label_check));
        bb_continue->AddSuccessor(bb_check);
        bb_check->AddPredecessor(bb_continue);
    }
    continue_stack.pop();

    for (const auto &bb_true : true_stack.top()) {
        bb_true->GetInstList().back()->Cast<ir::BrInst>().SetTrue(label_body);
        bb_true->AddSuccessor(bb_body);
        bb_body->AddPredecessor(bb_true);
    }
    true_stack.pop();

    for (const auto &bb_false : false_stack.top()) {
        bb_false->GetInstList().back()->Cast<ir::BrInst>().SetFalse(label_end);
        bb_false->AddSuccessor(bb_end);
        bb_end->AddPredecessor(bb_false);
    }
    false_stack.pop();

    return bb_end;
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
            ret_expr = TranslateExpr(def, bb, stmt.GetExpr(), tmp_id);
        }
        bb->AddInst(new ir::RetInst(ret_expr));
    } else {
        bb->AddInst(new ir::RetInst);
    }
}

std::shared_ptr<ir::Value> TranslateExpr(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::Expr &expr,
    int &tmp_id) {
    switch (expr.kind) {
        case ast::ASTNode::kParenExpr:
            return TranslateExpr(
                def, bb, expr.Cast<ast::ParenExpr>().GetSubExpr(), tmp_id);
        case ast::ASTNode::kDeclRefExpr:
            return TranslateDeclRefExpr(def, bb, expr.Cast<ast::DeclRefExpr>(),
                                        tmp_id, true);
        case ast::ASTNode::kCallExpr:
            return TranslateCallExpr(def, bb, expr.Cast<ast::CallExpr>(),
                                     tmp_id, true);
        case ast::ASTNode::kBinaryOperator:
            return TranslateBinaryOperator(
                def, bb, expr.Cast<ast::BinaryOperator>(), tmp_id);
        case ast::ASTNode::kUnaryOperator:
            return TranslatekUnaryOperator(
                def, bb, expr.Cast<ast::UnaryOperator>(), tmp_id);
        default:
            break;
    }
    return nullptr;
}

std::shared_ptr<ir::Value> TranslateDeclRefExpr(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::DeclRefExpr &expr,
    int &tmp_id,
    const bool need_load) {
    auto ptr = node_map.find(expr.GetRef().GetLocation())->second;
    auto addr = ptr;

    // func real arr param
    if (expr.ResultIsArr()) {
        // ref VarDecl
        if (expr.GetRef().kind == ast::ASTNode::kVarDecl
            && expr.GetRef().Cast<ast::VarDecl>().IsArray()) {
            std::vector<std::shared_ptr<ir::Value>> idx_list;
            idx_list.emplace_back(new ir::Imm(0));
            for (auto expr_loc : expr.GetArrDimList()) {
                const auto &expr = ast_manager.GetExpr(expr_loc);
                if (expr.IsConst()) {
                    idx_list.emplace_back(new ir::Imm(expr.GetValue()));
                } else {
                    idx_list.emplace_back(TranslateExpr(def, bb, expr, tmp_id));
                }
            }
            idx_list.emplace_back(new ir::Imm(0));

            auto arr_dim_list = ptr->GetType()
                                    .Cast<ir::PtrType>()
                                    .GetPointee()
                                    .Cast<ir::ArrayType>()
                                    .GetArrDimList();
            auto ref_dim = arr_dim_list.size();
            auto index_dim = expr.GetArrDimNum();
            if (ref_dim - index_dim == 1) {
                auto result
                    = std::make_shared<ir::TmpVar>(new ir::PtrType, tmp_id++);
                bb->AddInst(new ir::GetelementptrInst(result, ptr, idx_list));
                return result;
            }
            auto iter = arr_dim_list.cbegin();
            auto len = idx_list.size() - 1;
            while (len > 0) {
                ++iter;
                --len;
            }
            auto new_arr_type = std::make_shared<ir::ArrayType>(
                std::vector<int>{iter, arr_dim_list.cend()});
            auto result = std::make_shared<ir::TmpVar>(
                new ir::PtrType(new_arr_type), tmp_id++);
            bb->AddInst(new ir::GetelementptrInst(result, ptr, idx_list));
            return result;
        }
        // ref ParamVarDecl
        if (expr.GetRef().kind == ast::ASTNode::kParamVarDecl
            && expr.GetRef().Cast<ast::ParamVarDecl>().IsPtr()) {
            auto param_type
                = ptr->GetType().Cast<ir::PtrType>().GetPointeePtr();
            auto result = std::make_shared<ir::TmpVar>(param_type, tmp_id++);
            bb->AddInst(new ir::LoadInst(result, ptr));
            return result;
        }
    }

    if (expr.IsConst()) return std::make_shared<ir::Imm>(expr.GetValue());
    if (expr.IsArray()) {
        std::vector<std::shared_ptr<ir::Value>> idx_list;
        // VarDecl & ParamVarDecl
        if (expr.GetRef().kind == ast::ASTNode::kVarDecl) {
            idx_list.emplace_back(new ir::Imm(0));
        } else {
            auto new_ptr = std::make_shared<ir::TmpVar>(
                ptr->GetType().Cast<ir::PtrType>().GetPointeePtr(), tmp_id++);
            bb->AddInst(new ir::LoadInst(new_ptr, ptr));
            ptr = new_ptr;
        }
        for (auto expr_loc : expr.GetArrDimList()) {
            const auto &expr = ast_manager.GetExpr(expr_loc);
            if (expr.IsConst()) {
                idx_list.emplace_back(new ir::Imm(expr.GetValue()));
            } else {
                idx_list.emplace_back(TranslateExpr(def, bb, expr, tmp_id));
            }
        }
        addr = std::make_shared<ir::TmpVar>(new ir::PtrType, tmp_id++);
        bb->AddInst(new ir::GetelementptrInst(addr, ptr, idx_list));
    }

    if (need_load) {
        auto result = std::make_shared<ir::TmpVar>(tmp_id++);
        bb->AddInst(new ir::LoadInst(result, addr));
        return result;
    }
    return addr;
}

std::shared_ptr<ir::Value> TranslateCallExpr(
    const std::shared_ptr<ir::FuncDef> &def,
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
            param_list.emplace_back(TranslateExpr(def, bb, expr, tmp_id));
        }
    }
    if (has_ret) {
        auto result = std::make_shared<ir::TmpVar>(tmp_id++);
        bb->AddInst(new ir::CallInst(result, func, param_list));
        return result;
    }
    if (expr.GetRef().GetType() == ast::Decl::kInt) tmp_id++;
    bb->AddInst(new ir::CallInst(func, param_list));
    return nullptr;
}

std::shared_ptr<ir::Value> TranslateBinaryOperator(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::BinaryOperator &expr,
    int &tmp_id) {
    if (ast::BinaryOperator::kOr <= expr.op_code
        && expr.op_code <= ast::BinaryOperator::kAnd) {
        return TranslateCondOperator(def, bb, expr, tmp_id);
    }

    auto zero_i32 = std::make_shared<ir::Imm>(0);
    auto zero_i1 = std::make_shared<ir::Imm>(false);

    std::shared_ptr<ir::TmpVar> result;
    std::shared_ptr<ir::Value> lhs;
    std::shared_ptr<ir::Value> rhs;

    if (expr.GetLHS().IsConst()) {
        lhs.reset(new ir::Imm(expr.GetLHS().GetValue()));
    } else {
        lhs = TranslateExpr(def, bb, expr.GetLHS(), tmp_id);
    }
    if (expr.GetRHS().IsConst()) {
        rhs.reset(new ir::Imm(expr.GetRHS().GetValue()));
    } else {
        rhs = TranslateExpr(def, bb, expr.GetRHS(), tmp_id);
    }

    // format
    if (ast::BinaryOperator::kAdd <= expr.op_code
        && expr.op_code <= ast::BinaryOperator::kRem) {
        // lhs: i32
        if (lhs->GetType().Cast<ir::IntType>().GetWidth() == ir::IntType::kI1) {
            auto new_lhs = std::make_shared<ir::TmpVar>(tmp_id++);
            bb->AddInst(new ir::ZextInst(new_lhs, lhs));
            lhs = new_lhs;
        }
        // rhs: i32
        if (rhs->GetType().Cast<ir::IntType>().GetWidth() == ir::IntType::kI1) {
            auto new_rhs = std::make_shared<ir::TmpVar>(tmp_id++);
            bb->AddInst(new ir::ZextInst(new_rhs, rhs));
            rhs = new_rhs;
        }
        // result
        result = std::make_shared<ir::TmpVar>(tmp_id++);
    } else {
        // lhs: i32
        if (lhs->GetType().Cast<ir::IntType>().GetWidth() == ir::IntType::kI1) {
            auto new_lhs = std::make_shared<ir::TmpVar>(tmp_id++);
            bb->AddInst(new ir::ZextInst(new_lhs, lhs));
            lhs = new_lhs;
        }
        // rhs: i32
        if (rhs->GetType().Cast<ir::IntType>().GetWidth() == ir::IntType::kI1) {
            auto new_rhs = std::make_shared<ir::TmpVar>(tmp_id++);
            bb->AddInst(new ir::ZextInst(new_rhs, rhs));
            rhs = new_rhs;
        }
        // result
        result = std::make_shared<ir::TmpVar>(new ir::IntType(ir::IntType::kI1),
                                              tmp_id++);
    }

    switch (expr.op_code) {
        // i32 op i32, i32
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
        // i1 op i1 i1
        // case ast::BinaryOperator::kOr:
        //     bb->AddInst(new ir::BitwiseOpInst(ir::BitwiseOpInst::kOr, result,
        //                                       lhs, rhs));
        //     break;
        // case ast::BinaryOperator::kAnd:
        //     bb->AddInst(new ir::BitwiseOpInst(ir::BitwiseOpInst::kAnd,
        //     result,
        //                                       lhs, rhs));
        //     break;
        // i1 op i32 i32
        // i1 op i1 i1
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

std::shared_ptr<ir::Value> TranslateCondOperator(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::BinaryOperator &expr,
    int &tmp_id) {
    auto zero_i32 = std::make_shared<ir::Imm>(0);
    auto zero_i1 = std::make_shared<ir::Imm>(false);
    auto one_i1 = std::make_shared<ir::Imm>(true);

    bool pass_lhs = false;
    std::shared_ptr<ir::TmpVar> label_lhs;
    std::shared_ptr<ir::BasicBlock> bb_lhs;

    bool pass_rhs = false;
    std::shared_ptr<ir::TmpVar> label_rhs;
    std::shared_ptr<ir::BasicBlock> bb_rhs;

    if (expr.GetLHS().IsConst()) {
        if (expr.op_code == ast::BinaryOperator::kAnd
            && expr.GetLHS().GetValue() == 0) {
            return zero_i1;
        }
        if (expr.op_code == ast::BinaryOperator::kOr
            && expr.GetLHS().GetValue() != 0) {
            return one_i1;
        }
        pass_lhs = true;
    }

    if (expr.GetRHS().IsConst()) {
        if (expr.op_code == ast::BinaryOperator::kAnd
            && expr.GetRHS().GetValue() == 0) {
            return zero_i1;
        }
        if (expr.op_code == ast::BinaryOperator::kOr
            && expr.GetRHS().GetValue() != 0) {
            return one_i1;
        }
        pass_rhs = true;
    }

    if (pass_lhs) return TranslateExpr(def, bb, expr.GetRHS(), tmp_id);
    if (pass_rhs) return TranslateExpr(def, bb, expr.GetLHS(), tmp_id);

    // LHS
    true_stack.emplace();
    false_stack.emplace();

    label_lhs = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    bb_lhs = std::make_shared<ir::BasicBlock>(label_lhs);
    def->AddBlock(bb_lhs);
    bb->AddSuccessor(bb_lhs);
    bb_lhs->AddPredecessor(bb);
    // lhs: i1
    auto cond_lhs = TranslateExpr(def, bb_lhs, expr.GetLHS(), tmp_id);
    if (cond_lhs != nullptr
        && cond_lhs->GetType().Cast<ir::IntType>().GetWidth()
               == ir::IntType::kI32) {
        auto new_lhs = std::make_shared<ir::TmpVar>(
            new ir::IntType(ir::IntType::kI1), tmp_id++);
        bb_lhs->AddInst(
            new ir::IcmpInst(ir::IcmpInst::kNE, new_lhs, cond_lhs, zero_i32));
        cond_lhs = new_lhs;
    }

    std::list<std::shared_ptr<ir::BasicBlock>> lhs_true = true_stack.top();
    std::list<std::shared_ptr<ir::BasicBlock>> lhs_false = false_stack.top();
    true_stack.pop();
    false_stack.pop();

    // RHS
    true_stack.emplace();
    false_stack.emplace();

    label_rhs = std::make_shared<ir::TmpVar>(new ir::LabelType, tmp_id++);
    bb_rhs = std::make_shared<ir::BasicBlock>(label_rhs);
    def->AddBlock(bb_rhs);
    bb->AddSuccessor(bb_rhs);
    bb_rhs->AddPredecessor(bb);
    // rhs: i1
    auto cond_rhs = TranslateExpr(def, bb_rhs, expr.GetRHS(), tmp_id);
    if (cond_rhs != nullptr
        && cond_rhs->GetType().Cast<ir::IntType>().GetWidth()
               == ir::IntType::kI32) {
        auto new_rhs = std::make_shared<ir::TmpVar>(
            new ir::IntType(ir::IntType::kI1), tmp_id++);
        bb_rhs->AddInst(
            new ir::IcmpInst(ir::IcmpInst::kNE, new_rhs, cond_rhs, zero_i32));
        cond_rhs = new_rhs;
    }

    std::list<std::shared_ptr<ir::BasicBlock>> rhs_true = true_stack.top();
    std::list<std::shared_ptr<ir::BasicBlock>> rhs_false = false_stack.top();
    true_stack.pop();
    false_stack.pop();

    // Link
    bb->AddInst(new ir::BrInst(label_lhs));

    // Link LHS
    if (cond_lhs != nullptr) {
        bb_lhs->AddInst(new ir::BrInst(cond_lhs, label_rhs, label_rhs));
        if (expr.op_code == ast::BinaryOperator::kAnd) {
            false_stack.top().emplace_back(bb_lhs);
        } else {
            true_stack.top().emplace_back(bb_lhs);
        }
    } else {
        if (expr.op_code == ast::BinaryOperator::kAnd) {
            for (const auto &bb_lhs_sub : lhs_true) {
                bb_lhs_sub->GetInstList().back()->Cast<ir::BrInst>().SetTrue(
                    label_rhs);
            }
            for (const auto &bb_lhs_sub : lhs_false) {
                false_stack.top().emplace_back(bb_lhs_sub);
            }
        } else {
            for (const auto &bb_lhs_sub : lhs_true) {
                true_stack.top().emplace_back(bb_lhs_sub);
            }
            for (const auto &bb_lhs_sub : lhs_false) {
                bb_lhs_sub->GetInstList().back()->Cast<ir::BrInst>().SetFalse(
                    label_rhs);
            }
        }
    }

    // Link RHS
    if (cond_rhs != nullptr) {
        bb_rhs->AddInst(new ir::BrInst(cond_rhs, label_rhs, label_rhs));
        true_stack.top().emplace_back(bb_rhs);
        false_stack.top().emplace_back(bb_rhs);
    } else {
        for (const auto &bb_rhs_sub : rhs_true) {
            true_stack.top().emplace_back(bb_rhs_sub);
        }
        for (const auto &bb_rhs_sub : rhs_false) {
            false_stack.top().emplace_back(bb_rhs_sub);
        }
    }

    return nullptr;
}

void TranslateAssignOperator(const std::shared_ptr<ir::FuncDef> &def,
                             const std::shared_ptr<ir::BasicBlock> &bb,
                             const ast::BinaryOperator &expr,
                             int &tmp_id) {
    std::shared_ptr<ir::Value> value;
    if (expr.GetRHS().IsConst()) {
        value = std::make_shared<ir::Imm>(expr.GetRHS().GetValue());
    } else {
        value = TranslateExpr(def, bb, expr.GetRHS(), tmp_id);
    }

    auto addr = TranslateDeclRefExpr(
        def, bb, expr.GetLHS().Cast<ast::DeclRefExpr>(), tmp_id, false);
    bb->AddInst(new ir::StoreInst(value, addr));
}

std::shared_ptr<ir::Value> TranslatekUnaryOperator(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::UnaryOperator &expr,
    int &tmp_id) {
    std::shared_ptr<ir::TmpVar> result;
    std::shared_ptr<ir::Value> sub_expr;
    auto zero_i32 = std::make_shared<ir::Imm>(0);
    auto zero_i1 = std::make_shared<ir::Imm>(false);

    if (expr.GetSubExpr().IsConst()) {
        sub_expr.reset(new ir::Imm(expr.GetSubExpr().GetValue()));
    } else {
        sub_expr = TranslateExpr(def, bb, expr.GetSubExpr(), tmp_id);
    }

    std::shared_ptr<ir::Value> rhs = sub_expr;
    switch (expr.op_code) {
        case ast::UnaryOperator::kPlus:
            return sub_expr;
        case ast::UnaryOperator::kMinus:
            if (sub_expr->GetType().Cast<ir::IntType>().GetWidth()
                == ir::IntType::kI1) {
                rhs = std::make_shared<ir::TmpVar>(
                    new ir::IntType(ir::IntType::kI32), tmp_id++);
                bb->AddInst(new ir::ZextInst(rhs, sub_expr));
            }
            result = std::make_shared<ir::TmpVar>(
                new ir::IntType(ir::IntType::kI32), tmp_id++);
            bb->AddInst(new ir::BinaryOpInst(ir::BinaryOpInst::kSub, result,
                                             zero_i32, rhs));
            break;
        case ast::UnaryOperator::kNot:
            auto rhs = sub_expr->GetType().Cast<ir::IntType>().GetWidth()
                               == ir::IntType::kI32
                           ? zero_i32
                           : zero_i1;
            result = std::make_shared<ir::TmpVar>(
                new ir::IntType(ir::IntType::kI1), tmp_id++);
            bb->AddInst(
                new ir::IcmpInst(ir::IcmpInst::kEQ, result, sub_expr, rhs));
            break;
    }

    return result;
}

}  // namespace frontend
