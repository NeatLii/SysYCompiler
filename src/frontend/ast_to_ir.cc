#include <iostream>
#include <memory>
#include <string>

#include "frontend/ast_manager.h"
#include "frontend/frontend.h"
#include "ir/ir.h"
#include "ir/type.h"
#include "ir/value.h"

static auto *module = new ir::Module();

void TranslateGlobalVarDecl(const ast::VarDecl &decl);
void TranslateFunctionDecl(const ast::FunctionDecl &decl);

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
    return std::shared_ptr<ir::Module>(module);
}

void TranslateGlobalVarDecl(const ast::VarDecl &decl) {
    ir::Type *type = nullptr;
    if (decl.IsArray()) {
        std::vector<int> arr_dim_list;
        for (auto expr : decl.GetArrDimList()) {
            arr_dim_list.emplace_back(ast_manager.GetExpr(expr).GetValue());
        }
        type = new ir::ArrayType(new ir::IntType(ir::IntType::kI32),
                                 arr_dim_list);
    } else {
        type = new ir::IntType(ir::IntType::kI32);
    }

    auto *var = new ir::GlobalVar(type, decl.GetIdentName());

    std::vector<std::shared_ptr<ir::Imm>> init_list;
    if (decl.IsArray()) {
        if (!decl.HasInit() || decl.GetInitList().GetInitList().empty()) {
            module->AddVar(
                new ir::GlobalVarDef(var, decl.IsConst(), init_list, true));
        } else {
            for (auto init_val : decl.GetInitList().GetInitMap()) {
                init_list.emplace_back(new ir::Imm(init_val));
            }
            module->AddVar(
                new ir::GlobalVarDef(var, decl.IsConst(), init_list, false));
        }
    } else {
        init_list.emplace_back(
            new ir::Imm(decl.HasInit() ? decl.GetInit().GetValue() : 0));
        module->AddVar(
            new ir::GlobalVarDef(var, decl.IsConst(), init_list, false));
    }
}

void TranslateFunctionDecl(const ast::FunctionDecl &decl) {
    ir::Type *ret_type = nullptr;
    if (decl.GetType() == ast::Decl::kVoid) {
        ret_type = new ir::VoidType;
    } else {
        ret_type = new ir::IntType(ir::IntType::kI32);
    }

    std::vector<ir::Type *> param_list;
    for (auto param_loc : decl.GetParamList()) {
        const auto &param
            = ast_manager.GetNode(param_loc).Cast<ast::ParamVarDecl>();
        if (param.IsArrayPtr()) {
            std::vector<int> arr_dim_list;
            for (auto expr : param.GetArrDimList()) {
                arr_dim_list.emplace_back(ast_manager.GetExpr(expr).GetValue());
            }
            auto *arr = new ir::ArrayType(new ir::IntType(ir::IntType::kI32),
                                          arr_dim_list);
            param_list.emplace_back(new ir::PtrType(arr));
        } else if (param.IsPtr()) {
            param_list.emplace_back(
                new ir::PtrType(new ir::IntType(ir::IntType::kI32)));
        } else {
            param_list.emplace_back(new ir::IntType(ir::IntType::kI32));
        }
    }

    auto *var = new ir::GlobalVar(new ir::FuncType(ret_type, param_list),
                                  decl.GetIdentName());

    if (decl.HasDef()) {
    } else {
        module->AddFuncDecl(new ir::FuncDecl(var));
    }
}
