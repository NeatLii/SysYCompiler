#include "frontend/ast_manager.h"

#include <error.h>

#include <array>
#include <memory>
#include <sstream>
#include <string>

#include "frontend/source_manager.h"
#include "util.h"

namespace ast {

/* class ASTManager */

ASTLocation ASTManager::AddNode(ASTNode *node) {
    node_table.emplace_back(node);
    node->SetLocation(node_table.size() - 1);
    return node_table.size() - 1;
}

ASTLocation ASTManager::AddNode(std::unique_ptr<ASTNode> &node) {
    node_table.emplace_back(node.release());
    node->SetLocation(node_table.size() - 1);
    return node_table.size() - 1;
}

void ASTManager::SetRoot(const ASTLocation root) {
    has_root = true;
    this->root = root;
    GetRoot().SetLocation(root);
}
void ASTManager::SetRoot(ASTNode *root) {
    has_root = true;
    this->root = AddNode(root);
    GetRoot().SetLocation(this->root);
}
void ASTManager::SetRoot(std::unique_ptr<ASTNode> &root) {
    has_root = true;
    this->root = AddNode(root);
    GetRoot().SetLocation(this->root);
}

TranslationUnit &ASTManager::GetRoot() const {
    return node_table[root]->Cast<TranslationUnit &>();
}

Decl &ASTManager::GetDecl(const ASTLocation loc) const {
    return node_table[loc]->Cast<Decl &>();
}

Stmt &ASTManager::GetStmt(const ASTLocation loc) const {
    return node_table[loc]->Cast<Stmt &>();
}

Expr &ASTManager::GetExpr(const ASTLocation loc) const {
    return node_table[loc]->Cast<Expr &>();
}

void ASTManager::Dump(std::ostream &ostream) const {
    ostream << util::FormatTerminal("Dump AST from file", util::kFGBrightGreen,
                                    util::kBGDefault,
                                    {util::kBold, util::kUnderLine});
    ostream << " '" << util::FormatTerminal(raw.GetFileName(), util::kFGYellow)
            << '\'';
    ostream << ", "
            << util::FormatTerminalBold("AST Node count", util::kFGBrightGreen)
            << ' ' << node_table.size();
    ostream << std::endl;
    node_table[root]->Dump(ostream, "", true);
}
/* class IdentTable */

std::pair<bool, ASTLocation> IdentTable::FindIdent(
    const std::string &name) const {
    auto iter = ident_table.find(name);
    if (iter == ident_table.end()) return {false, 0};
    return {true, iter->second};
}

void IdentTable::DumpIdentTable(std::ostream &ostream) const {
    for (const auto &pair : ident_table) {
        ostream << util::FormatTerminal(util::FormatHex32(pair.second),
                                        util::kFGYellow);
        ostream << ' '
                << util::FormatTerminalBold(pair.first, util::kFGBrightGreen);
        ostream << std::endl;
    }
}

/* class ASTNode */

void ASTNode::DumpIndentAndBranch(std::ostream &ostream,
                                  const std::string &indent,
                                  const bool is_last) {
    ostream << util::FormatTerminal(indent + (is_last ? "`-" : "|-"),
                                    util::kFGBlue);
}

void ASTNode::DumpInfo(std::ostream &ostream, const std::string &kind) const {
    // node kind
    ostream << util::FormatTerminalBold(
        kind, IsDecl() ? util::kFGBrightGreen : util::kFGBrightMagenta);
    // node location
    ostream << ' '
            << util::FormatTerminal(util::FormatHex32(loc), util::kFGYellow);
    // node source range
    ostream << " <" << util::FormatTerminal(range.DumpBegin(), util::kFGYellow)
            << ", " << util::FormatTerminal(range.DumpEnd(), util::kFGYellow)
            << '>';
}

/* class TranslationUnit */

void TranslationUnit::AddDecl(const ASTLocation loc) {
    decl_list.emplace_back(loc);
    ident_table.emplace(src.GetDecl(loc).GetIdentName(), loc);
}

void TranslationUnit::Dump(std::ostream &ostream,
                           const std::string &indent,
                           const bool is_last) const {
    DumpInfo(ostream, "TranslationUnit");
    // newline
    ostream << std::endl;
    // decl list
    if (decl_list.empty()) return;
    auto iter = decl_list.cbegin();
    for (; iter != decl_list.cend() - 1; ++iter) {
        src.GetDecl(*iter).Dump(ostream, "", false);
    }
    src.GetDecl(*iter).Dump(ostream, "", true);
}

/* class Decl */

/* class VarDecl */

std::string VarDecl::TypeStr() const {
    std::string type_str = "int";
    for (auto expr : arr_dim_list) {
        type_str += '[' + std::to_string(src.GetExpr(expr).GetValue()) + ']';
    }
    return is_const ? "const " + type_str : type_str;
}

void VarDecl::Dump(std::ostream &ostream,
                   const std::string &indent,
                   const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "VarDecl");
    // var name
    ostream << ' ' << GetIdentToken().DumpText();
    // var type
    ostream << ' '
            << util::FormatTerminal('\'' + TypeStr() + '\'', util::kFGGreen);
    // var name's source range
    ostream << ' ' << GetIdentToken().DumpRange();
    // newline
    ostream << std::endl;
    // init
    if (!has_init) return;
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    GetInit().Dump(ostream, child_indent, true);
}

/* class ParamVarDecl */

std::string ParamVarDecl::TypeStr() const {
    std::string type_str = "int";
    if (is_ptr) {
        if (arr_dim_list.empty()) {
            type_str += " *";
        } else {
            type_str += " (*)";
            for (auto expr : arr_dim_list) {
                type_str
                    += '[' + std::to_string(src.GetExpr(expr).GetValue()) + ']';
            }
        }
    }
    return type_str;
}

void ParamVarDecl::Dump(std::ostream &ostream,
                        const std::string &indent,
                        const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "ParamVarDecl");
    // var name
    ostream << ' ' << GetIdentToken().DumpText();
    // var type
    ostream << ' '
            << util::FormatTerminal('\'' + TypeStr() + '\'', util::kFGGreen);
    // var name's source range
    ostream << ' ' << GetIdentToken().DumpRange();
    // newline
    ostream << std::endl;
}

/* class FunctionDecl */

std::string FunctionDecl::TypeStr() const {
    std::string type_str = type == kVoid ? "void (" : "int (";
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) type_str += ", ";
        type_str += src.GetNode(*iter).Cast<const ParamVarDecl &>().TypeStr();
    }
    return type_str + ')';
}

void FunctionDecl::Dump(std::ostream &ostream,
                        const std::string &indent,
                        const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "FunctionDecl");
    // func name
    ostream << ' ' << GetIdentToken().DumpText();
    // func type
    ostream << ' '
            << util::FormatTerminal('\'' + TypeStr() + '\'', util::kFGGreen);
    // func name's source range
    ostream << ' ' << GetIdentToken().DumpRange();
    // newline
    ostream << std::endl;
    // param list
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    if (!param_list.empty()) {
        auto iter = param_list.cbegin();
        for (; iter != param_list.cend() - 1; ++iter) {
            src.GetNode(*iter).Dump(ostream, child_indent, false);
        }
        src.GetNode(*iter).Dump(ostream, child_indent, !has_def);
    }
    // def
    if (has_def) GetDef().Dump(ostream, child_indent, true);
}

/* class Stmt */

/* class CompoundStmt */

void CompoundStmt::Dump(std::ostream &ostream,
                        const std::string &indent,
                        const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "CompoundStmt");
    // newline
    ostream << std::endl;
    // stmt list
    if (!stmt_list.empty()) {
        const std::string child_indent = indent + (is_last ? "  " : "| ");
        auto iter = stmt_list.cbegin();
        for (; iter != stmt_list.cend() - 1; ++iter) {
            src.GetStmt(*iter).Dump(ostream, child_indent, false);
        }
        src.GetStmt(*iter).Dump(ostream, child_indent, true);
    }
}

/* class DeclStmt */

void DeclStmt::Dump(std::ostream &ostream,
                    const std::string &indent,
                    const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "DeclStmt");
    // newline
    ostream << std::endl;
    // decl list
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    auto iter = decl_list.cbegin();
    for (; iter != decl_list.cend() - 1; ++iter) {
        src.GetDecl(*iter).Dump(ostream, child_indent, false);
    }
    src.GetDecl(*iter).Dump(ostream, child_indent, true);
}

/* class NullStmt */

void NullStmt::Dump(std::ostream &ostream,
                    const std::string &indent,
                    const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "NullStmt");
    // newline
    ostream << std::endl;
}

/* class IfStmt */

void IfStmt::Dump(std::ostream &ostream,
                  const std::string &indent,
                  const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "IfStmt");
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    // newline
    ostream << std::endl;
    // cond
    GetCond().Dump(ostream, child_indent, false);
    // then
    GetThen().Dump(ostream, child_indent, !has_else);
    // else
    if (has_else) GetElse().Dump(ostream, child_indent, true);
}

/* class WhileStmt */

void WhileStmt::Dump(std::ostream &ostream,
                     const std::string &indent,
                     const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "WhileStmt");
    // newline
    ostream << std::endl;
    // cond
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    GetCond().Dump(ostream, child_indent, false);
    // then
    GetBody().Dump(ostream, child_indent, true);
}

/* class ContinueStmt */

void ContinueStmt::Dump(std::ostream &ostream,
                        const std::string &indent,
                        const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "ContinueStmt");
    // newline
    ostream << std::endl;
}

/* class BreakStmt */

void BreakStmt::Dump(std::ostream &ostream,
                     const std::string &indent,
                     const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "BreakStmt");
    // newline
    ostream << std::endl;
}

/* class ReturnStmt */

void ReturnStmt::Dump(std::ostream &ostream,
                      const std::string &indent,
                      const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "ReturnStmt");
    // newline
    ostream << std::endl;
    // expr
    if (has_expr) {
        const std::string child_indent = indent + (is_last ? "  " : "| ");
        GetExpr().Dump(ostream, child_indent, true);
    }
}

/* class Expr */

void Expr::DumpConstExpr(std::ostream &ostream) const {
    if (is_const) {
        ostream << " const expr "
                << util::FormatTerminalBold(std::to_string(value),
                                            util::kFGBrightCyan);
    }
}

/* class IntegerLiteral */

void IntegerLiteral::Dump(std::ostream &ostream,
                          const std::string &indent,
                          const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "IntegerLiteral");
    // value type
    ostream << ' ' << util::FormatTerminal("'int'", util::kFGGreen);
    // value
    DumpConstExpr(ostream);
    // newline
    ostream << std::endl;
}

/* class ParenExpr */

ParenExpr::ParenExpr(const ASTManager &src,
                     SourceRange range,
                     const ASTLocation sub_expr)
    : Expr(kParenExpr, src, range), sub_expr(sub_expr) {
    const Expr &expr = GetSubExpr();
    is_const = expr.IsConst();
    value = expr.GetValue();
}

void ParenExpr::Dump(std::ostream &ostream,
                     const std::string &indent,
                     const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "ConstExpr");
    // value type
    ostream << ' ' << util::FormatTerminal("'int'", util::kFGGreen);
    // value
    DumpConstExpr(ostream);
    // newline
    ostream << std::endl;
    // expr
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    GetSubExpr().Dump(ostream, child_indent, true);
}

/* class DeclRefExpr */

DeclRefExpr::DeclRefExpr(const ASTManager &src,
                         SourceRange range,
                         const TokenLocation ident,
                         std::vector<ASTLocation> arr_dim_list,
                         const bool has_ref,
                         const ASTLocation ref)
    : Expr(kDeclRefExpr, src, range)
    , ident(ident)
    , arr_dim_list(std::move(arr_dim_list))
    , has_ref(has_ref)
    , ref(ref) {
    for (auto expr : arr_dim_list) {
        if (!src.GetExpr(expr).IsConst()) return;
    }
    is_const = true;
}

void DeclRefExpr::SetRef(const ASTLocation ref) {
    has_ref = true;
    this->ref = ref;
    // if (!is_const || GetRef().kind == kParamVarDecl
    //     || !src.GetNode(ref).Cast<VarDecl>().IsConst()) {
    //     is_const = false;
    //     return;
    // }
    // // calculate value
    // if (arr_dim_list.empty()) {
    //     value = src.GetNode(ref).Cast<VarDecl>().GetInit().GetValue();
    //     return;
    // }
    // const Expr &init = src.GetNode(ref).Cast<VarDecl>().GetInit();
    // for (auto expr : arr_dim_list) {
    //     init
    //         =
    //         init.Cast<InitListExpr>().GetInitAt(src.GetExpr(expr).GetValue());
    // }
    // value = init.GetValue();
}

std::string DeclRefExpr::TypeStr() const {
    std::string type_str = "int";
    for (std::vector<ASTLocation>::size_type i = 0; i < arr_dim_list.size();
         ++i) {
        type_str += "[]";
    }
    return type_str;
}

void DeclRefExpr::Dump(std::ostream &ostream,
                       const std::string &indent,
                       const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "DeclRefExpr");
    // type
    ostream << ' '
            << util::FormatTerminal('\'' + TypeStr() + '\'', util::kFGGreen);
    // lvalue var ref
    ostream << ' ' << util::FormatTerminal("lvalue Var", util::kFGCyan);
    // ref location
    ostream << ' '
            << util::FormatTerminal(
                   has_ref ? util::FormatHex32(ref) : "unknown",
                   util::kFGYellow);
    // ref name
    ostream << ' ' << GetIdentToken().DumpTextRef();
    // ref type
    ostream << ' '
            << util::FormatTerminal(
                   has_ref ? ('\'' + GetRef().TypeStr() + '\'') : "'unknown'",
                   util::kFGGreen);
    // newline
    ostream << std::endl;
}

/* class CallExpr */

std::string CallExpr::TypeStr() const {
    std::string type_str;
    if (has_ref) {
        type_str = GetRef().TypeStr();
    } else {
        type_str = "unknown (";
        for (std::vector<ASTLocation>::size_type i = 0; i < param_list.size();
             ++i) {
            if (i != 0) type_str += ", ";
            type_str += "int";
        }
        type_str += ')';
    }
    return type_str;
}

void CallExpr::Dump(std::ostream &ostream,
                    const std::string &indent,
                    const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "CallExpr");
    // type
    ostream << ' '
            << util::FormatTerminal('\'' + TypeStr() + '\'', util::kFGGreen);
    // function ref
    ostream << ' ' << util::FormatTerminal("function", util::kFGCyan);
    // ref location
    ostream << ' '
            << util::FormatTerminal(
                   has_ref ? util::FormatHex32(ref) : "unknown",
                   util::kFGYellow);
    // ref name
    ostream << ' ' << GetIdentToken().DumpTextRef();
    // ref type
    ostream << ' '
            << util::FormatTerminal(
                   has_ref ? ('\'' + GetRef().TypeStr() + '\'') : "'unknown'",
                   util::kFGGreen);
    // newline
    ostream << std::endl;
}

/* class BinaryOperator */

BinaryOperator::BinaryOperator(const ASTManager &src,
                               const SourceRange range,
                               const BinaryOpKind op_code,
                               const ASTLocation LHS,
                               const ASTLocation RHS)
    : Expr(kBinaryOperator, src, range), op_code(op_code), LHS(LHS), RHS(RHS) {
    if (op_code < kAdd || kAssign < op_code) {
        std::stringstream dump;
        GetLHS().Dump(dump, "", true);
        dump << "\nop_code: " << std::to_string(op_code) << '\n';
        GetRHS().Dump(dump, "", true);
        throw InvalidOperatorException(dump.str());
    }
    const Expr &lhs = GetLHS();
    const Expr &rhs = GetRHS();
    if (lhs.IsConst() && rhs.IsConst()) {
        is_const = true;
        switch (op_code) {
            case kAdd:
                value = lhs.GetValue() + rhs.GetValue();
                break;
            case kSub:
                value = lhs.GetValue() - rhs.GetValue();
                break;
            case kMul:
                value = lhs.GetValue() * rhs.GetValue();
                break;
            case kDiv:
                value = lhs.GetValue() / rhs.GetValue();
                break;
            case kRem:
                value = lhs.GetValue() % rhs.GetValue();
                break;
            case kOr:
                value = static_cast<int>((lhs.GetValue() != 0)
                                         || (rhs.GetValue() != 0));
                break;
            case kAnd:
                value = static_cast<int>((lhs.GetValue() != 0)
                                         && (rhs.GetValue() != 0));
                break;
            case kEQ:
                value = static_cast<int>(lhs.GetValue() == rhs.GetValue());
                break;
            case kNE:
                value = static_cast<int>(lhs.GetValue() != rhs.GetValue());
                break;
            case kLT:
                value = static_cast<int>(lhs.GetValue() < rhs.GetValue());
                break;
            case kLE:
                value = static_cast<int>(lhs.GetValue() <= rhs.GetValue());
                break;
            case kGT:
                value = static_cast<int>(lhs.GetValue() > rhs.GetValue());
                break;
            case kGE:
                value = static_cast<int>(lhs.GetValue() >= rhs.GetValue());
                break;
            case kAssign:
                value = rhs.GetValue();
                break;
        }
    }
}

void BinaryOperator::Dump(std::ostream &ostream,
                          const std::string &indent,
                          const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "BinaryOperator");
    // value type
    ostream << ' ' << util::FormatTerminal("'int'", util::kFGGreen);
    // op
    static std::array<std::string, kAssign + 1> binary_op{
        "'+'",  "'-'",  "'*'", "'/'",  "'%'", "'||'", "'&&'",
        "'=='", "'!='", "'<'", "'<='", "'>'", "'>='", "'='"};
    ostream << ' ' << binary_op[op_code];
    // value
    DumpConstExpr(ostream);
    // newline
    ostream << std::endl;
    // LHS and RHS
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    GetLHS().Dump(ostream, child_indent, false);
    GetRHS().Dump(ostream, child_indent, true);
}

/* class UnaryOperator */

UnaryOperator::UnaryOperator(const ASTManager &src,
                             const SourceRange range,
                             const UnaryOpKind op_code,
                             const ASTLocation sub_expr)
    : Expr(kUnaryOperator, src, range), op_code(op_code), sub_expr(sub_expr) {
    if (op_code < kPlus || kNot < op_code) {
        std::stringstream dump;
        dump << "op_code: " << std::to_string(op_code) << '\n';
        GetSubExpr().Dump(dump, "", true);
        throw InvalidOperatorException(dump.str());
    }
    const Expr &sub = GetSubExpr();
    if (sub.IsConst()) {
        is_const = true;
        switch (op_code) {
            case kPlus:
                value = sub.GetValue();
                break;
            case kMinus:
                value = -sub.GetValue();
                break;
            case kNot:
                value = static_cast<int>(sub.GetValue() == 0);
                break;
        }
    }
}

void UnaryOperator::Dump(std::ostream &ostream,
                         const std::string &indent,
                         const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "UnaryOperator");
    // value type
    ostream << ' ' << util::FormatTerminal("'int'", util::kFGGreen);
    // op
    static std::array<std::string, kNot + 1> unary_op{"'+'", "'-'", "'!'"};
    ostream << " prefix " << unary_op[op_code];
    // value
    DumpConstExpr(ostream);
    // newline
    ostream << std::endl;
    // sub expr
    const std::string child_indent = indent + (is_last ? "  " : "| ");
    GetSubExpr().Dump(ostream, child_indent, true);
}

/* class InitListExpr */

InitListExpr::InitListExpr(const ASTManager &src, std::vector<ASTLocation> list)
    : Expr(kInitListExpr, src, range), init_list(std::move(list)) {
    for (auto init : init_list) {
        if (!src.GetExpr(init).IsConst()) return;
    }
    is_const = true;
}

void InitListExpr::Dump(std::ostream &ostream,
                        const std::string &indent,
                        const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    DumpInfo(ostream, "ArrayInitList");
    // newline
    ostream << std::endl;
    // init list
    if (!init_list.empty()) {
        const std::string child_indent = indent + (is_last ? "  " : "| ");
        auto iter = init_list.cbegin();
        for (; iter != init_list.cend() - 1; ++iter) {
            src.GetExpr(*iter).Dump(ostream, child_indent, false);
        }
        src.GetExpr(*iter).Dump(ostream, child_indent, true);
    }
}

}  // namespace ast
