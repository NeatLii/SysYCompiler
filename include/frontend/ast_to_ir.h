#ifndef __sysycompiler_frontend_ast_to_ir_h__
#define __sysycompiler_frontend_ast_to_ir_h__

#include "frontend/ast_manager.h"
#include "ir/ir.h"

namespace frontend {

/* decl node to ir */

void TranslateGlobalVarDecl(const ast::VarDecl &decl);
void TranslateLocalVarDecl(const std::shared_ptr<ir::FuncDef> &def,
                           const std::shared_ptr<ir::BasicBlock> &bb,
                           const ast::VarDecl &decl,
                           int &tmp_id);
void TranslateParamVarDecl(const std::shared_ptr<ir::BasicBlock> &bb,
                           const ast::ParamVarDecl &decl,
                           int &tmp_id);
void TranslateFunctionDecl(const ast::FunctionDecl &decl);

/* stmt node to ir */

std::shared_ptr<ir::BasicBlock> TranslateStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::Stmt &stmt,
    int &tmp_id);
std::shared_ptr<ir::BasicBlock> TranslatCompoundStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::CompoundStmt &stmt,
    int &tmp_id);
void TranslateDeclStmt(const std::shared_ptr<ir::FuncDef> &def,
                       const std::shared_ptr<ir::BasicBlock> &bb,
                       const ast::DeclStmt &stmt,
                       int &tmp_id);
std::shared_ptr<ir::BasicBlock> TranslateIfStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::IfStmt &stmt,
    int &tmp_id);
std::shared_ptr<ir::BasicBlock> TranslateWhileStmt(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::WhileStmt &stmt,
    int &tmp_id);
void TranslateReturnStmt(const std::shared_ptr<ir::FuncDef> &def,
                         const std::shared_ptr<ir::BasicBlock> &bb,
                         const ast::ReturnStmt &stmt,
                         int &tmp_id);

/* expr node to ir */
std::shared_ptr<ir::Value> TranslateExpr(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::Expr &expr,
    int &tmp_id);
std::shared_ptr<ir::Value> TranslateDeclRefExpr(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::DeclRefExpr &expr,
    int &tmp_id,
    bool need_load);
std::shared_ptr<ir::Value> TranslateCallExpr(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::CallExpr &expr,
    int &tmp_id,
    bool has_ret);
std::shared_ptr<ir::Value> TranslateBinaryOperator(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::BinaryOperator &expr,
    int &tmp_id);
std::shared_ptr<ir::Value> TranslateCondOperator(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::BinaryOperator &expr,
    int &tmp_id);
void TranslateAssignOperator(const std::shared_ptr<ir::FuncDef> &def,
                             const std::shared_ptr<ir::BasicBlock> &bb,
                             const ast::BinaryOperator &expr,
                             int &tmp_id);
std::shared_ptr<ir::Value> TranslatekUnaryOperator(
    const std::shared_ptr<ir::FuncDef> &def,
    const std::shared_ptr<ir::BasicBlock> &bb,
    const ast::UnaryOperator &expr,
    int &tmp_id);

}  // namespace frontend

#endif
