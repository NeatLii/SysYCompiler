#include "frontend/ast_manager.h"

#include <sstream>
#include <vector>

#include "error.h"
#include "util.h"

namespace ast {

/* class ASTManager */

ASTLocation ASTManager::AddNode(ASTNode *node) {
    node_table.emplace_back(node);
    node->SetLocation(node_table.size() - 1);
    node->Link();
    return node_table.size() - 1;
}

ASTLocation ASTManager::AddNode(std::unique_ptr<ASTNode> &node) {
    node_table.emplace_back(node.release());
    node->SetLocation(node_table.size() - 1);
    node->Link();
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
            << util::FormatTerminalBold("AST node count", util::kFGBrightGreen)
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
            << util::FormatTerminal(util::FormatHex32(location),
                                    util::kFGYellow);
    // node source range
    ostream << " <" << util::FormatTerminal(range.DumpBegin(), util::kFGYellow)
            << ", " << util::FormatTerminal(range.DumpEnd(), util::kFGYellow)
            << '>';
}

/* class TranslationUnit */

void TranslationUnit::AddDecl(const ASTLocation loc) {
    src.GetNode(loc).SetParent(this->location);
    decl_list.emplace_back(loc);
    ident_table.emplace(src.GetDecl(loc).GetIdentName(), loc);
}

void TranslationUnit::Visit() {
    for (auto decl : decl_list) src.GetNode(decl).Visit();
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

void TranslationUnit::BuiltIn() {
    SourceManager &raw = GetSourceManager();
    auto *getint = new FunctionDecl(src, {}, raw.AddToken("getint", {}));
    auto *getch = new FunctionDecl(src, {}, raw.AddToken("getch", {}));
    auto *getarray = new FunctionDecl(
        src, {}, raw.AddToken("getarray", {}),
        std::vector<ASTLocation>{src.AddNode(
            new ParamVarDecl(src, {}, raw.AddToken("a", {}), true))});
    auto *putint = new FunctionDecl(
        src, {}, raw.AddToken("putint", {}),
        std::vector<ASTLocation>{
            src.AddNode(new ParamVarDecl(src, {}, raw.AddToken("a", {})))});
    auto *putch = new FunctionDecl(
        src, {}, raw.AddToken("putch", {}),
        std::vector<ASTLocation>{
            src.AddNode(new ParamVarDecl(src, {}, raw.AddToken("a", {})))});
    auto *putarray = new FunctionDecl(
        src, {}, raw.AddToken("putarray", {}),
        {src.AddNode(new ParamVarDecl(src, {}, raw.AddToken("n", {}))),
         src.AddNode(new ParamVarDecl(src, {}, raw.AddToken("a", {}), true))});
    auto *_sysy_starttime = new FunctionDecl(
        src, {}, raw.AddToken("_sysy_starttime", {}),
        std::vector<ASTLocation>{src.AddNode(
            new ParamVarDecl(src, {}, raw.AddToken("lineno", {})))});
    auto *_sysy_stoptime = new FunctionDecl(
        src, {}, raw.AddToken("_sysy_stoptime", {}),
        std::vector<ASTLocation>{src.AddNode(
            new ParamVarDecl(src, {}, raw.AddToken("lineno", {})))});

    getint->SetType(Decl::kInt);
    getch->SetType(Decl::kInt);
    getarray->SetType(Decl::kInt);
    putint->SetType(Decl::kVoid);
    putch->SetType(Decl::kVoid);
    putarray->SetType(Decl::kVoid);
    _sysy_starttime->SetType(Decl::kVoid);
    _sysy_stoptime->SetType(Decl::kVoid);
    AddDecl(src.AddNode(getint));
    AddDecl(src.AddNode(getch));
    AddDecl(src.AddNode(getarray));
    AddDecl(src.AddNode(putint));
    AddDecl(src.AddNode(putch));
    AddDecl(src.AddNode(putarray));
    AddDecl(src.AddNode(_sysy_starttime));
    AddDecl(src.AddNode(_sysy_stoptime));
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

void VarDecl::Link() {
    for (auto expr : arr_dim_list) src.GetNode(expr).SetParent(location);
    if (has_init) src.GetNode(init).SetParent(location);
}

void VarDecl::Visit() {
    std::vector<int> format;
    for (auto expr : arr_dim_list) {
        src.GetNode(expr).Visit();
        format.emplace_back(src.GetExpr(expr).GetValue());
    }
    if (has_init) {
        src.GetNode(init).Visit();
        // format arr init list
        if (IsArray()) {
            auto &old_init = src.GetNode(init).Cast<InitListExpr>();
            init = InitListExpr::Format(src, old_init.GetRange(),
                                        old_init.GetInitList(), format);
            src.GetNode(init).SetParent(location);
        }
    }
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

void ParamVarDecl::Link() {
    for (auto expr : arr_dim_list) src.GetNode(expr).SetParent(location);
}

void ParamVarDecl::Visit() {
    for (auto expr : arr_dim_list) src.GetNode(expr).Visit();
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

void FunctionDecl::SetDef(const ASTLocation def) {
    has_def = true;
    this->def = def;
    src.GetNode(def).SetParent(location);
    Link();
}

std::string FunctionDecl::TypeStr() const {
    std::string type_str = type == kVoid ? "void (" : "int (";
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) type_str += ", ";
        type_str += src.GetNode(*iter).Cast<const ParamVarDecl &>().TypeStr();
    }
    return type_str + ')';
}

void FunctionDecl::Link() {
    for (auto decl : param_list) src.GetNode(decl).SetParent(location);
    if (has_def) {
        auto &body = src.GetNode(def).Cast<CompoundStmt>();
        body.SetParent(location);
        for (auto decl : param_list) {
            body.AddIdent(src.GetDecl(decl).GetIdentName(), decl);
        }
    }
}

void FunctionDecl::Visit() {
    for (auto decl : param_list) src.GetNode(decl).Visit();
    if (has_def) src.GetNode(def).Visit();
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

void CompoundStmt::Link() {
    for (auto stmt : stmt_list) {
        auto &child = src.GetNode(stmt);
        child.SetParent(location);
        if (child.kind == kDeclStmt) {
            for (auto decl : child.Cast<DeclStmt>().GetDeclList()) {
                ident_table.emplace(src.GetDecl(decl).GetIdentName(), decl);
            }
        }
    }
}

void CompoundStmt::Visit() {
    for (auto stmt : stmt_list) src.GetNode(stmt).Visit();
}

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

void DeclStmt::Link() {
    for (auto decl : decl_list) src.GetNode(decl).SetParent(location);
}

void DeclStmt::Visit() {
    for (auto decl : decl_list) src.GetNode(decl).Visit();
}

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

void IfStmt::Link() {
    src.GetNode(cond).SetParent(location);
    src.GetNode(then_stmt).SetParent(location);
    if (has_else) src.GetNode(else_stmt).SetParent(location);
}

void IfStmt::Visit() {
    src.GetNode(cond).Visit();
    src.GetNode(then_stmt).Visit();
    if (has_else) src.GetNode(else_stmt).Visit();
}

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

void WhileStmt::Link() {
    src.GetNode(cond).SetParent(location);
    src.GetNode(body).SetParent(location);
}

void WhileStmt::Visit() {
    src.GetNode(cond).Visit();
    src.GetNode(body).Visit();
}

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

void ReturnStmt::Link() {
    if (has_expr) src.GetNode(expr).SetParent(location);
}

void ReturnStmt::Visit() {
    if (has_expr) src.GetNode(expr).Visit();
}

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
    if (is_filler) {
        ostream << util::FormatTerminal("array_filler:", util::kFGBlue) << ' ';
    }
    DumpInfo(ostream, "IntegerLiteral");
    // value type
    ostream << ' ' << util::FormatTerminal("'int'", util::kFGGreen);
    // value
    DumpConstExpr(ostream);
    // newline
    ostream << std::endl;
}

/* class ParenExpr */

void ParenExpr::Link() { src.GetNode(sub_expr).SetParent(location); }

void ParenExpr::Visit() {
    Expr &expr = src.GetExpr(sub_expr);
    expr.Visit();
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

std::string DeclRefExpr::TypeStr() const {
    std::string type_str = "int";
    for (std::vector<ASTLocation>::size_type i = 0; i < arr_dim_list.size();
         ++i) {
        type_str += "[]";
    }
    return type_str;
}

void DeclRefExpr::Link() {
    for (auto expr : arr_dim_list) src.GetNode(expr).SetParent(location);
}

void DeclRefExpr::Visit() {
    for (auto expr : arr_dim_list) src.GetNode(expr).Visit();
    FindRef();
    CalculateValue();
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
    // value
    DumpConstExpr(ostream);
    // newline
    ostream << std::endl;
    // arr dim
    if (!arr_dim_list.empty()) {
        const std::string child_indent = indent + (is_last ? "  " : "| ");
        auto iter = arr_dim_list.cbegin();
        for (; iter != arr_dim_list.cend() - 1; ++iter) {
            src.GetExpr(*iter).Dump(ostream, child_indent, false);
        }
        src.GetExpr(*iter).Dump(ostream, child_indent, true);
    }
}

void DeclRefExpr::FindRef() {
    const std::string name = GetIdentName();
    ASTLocation cur = parent;
    while (!has_ref) {
        ASTNode &cur_node = src.GetNode(cur);
        auto result = cur_node.FindIdent(name);
        /*
         *  int a = 7;           // TokenLocation 'a' = 1
         *  int func() {
         *      int b = a;       // TokenLocation 'b' = 11, 'a' = 13
         *      int a = 1;       // TokenLocation 'a' = 16
         *  }
         *  // bug: int c = c;
         */
        if (result.first && ident > src.GetDecl(result.second).GetIdentLoc()) {
            has_ref = true;
            ref = result.second;
            continue;
        }
        if (cur_node.HasParent()) {
            cur = cur_node.GetParent();
            continue;
        }
        throw IdentRefNotFindException(GetIdentRange().Dump(), GetIdentName());
    }
}

void DeclRefExpr::CalculateValue() {
    /*
     *   int a;
     *   b[10];     // possible const
     *   b[a];      // impossible const
     */
    for (auto expr : this->arr_dim_list) {
        if (!src.GetExpr(expr).IsConst()) return;
    }
    if (GetRef().kind == kParamVarDecl
        || !src.GetNode(ref).Cast<VarDecl>().IsConst()) {
        return;
    }
    is_const = true;
    if (arr_dim_list.empty()) {
        value = src.GetNode(ref).Cast<VarDecl>().GetInit().GetValue();
    } else {
        value = InitListExpr::GetInitValue(
            src, src.GetNode(ref).Cast<VarDecl>().GetInitList(), arr_dim_list);
    }
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

void CallExpr::Link() {
    for (auto expr : param_list) src.GetNode(expr).SetParent(location);
}

void CallExpr::Visit() {
    for (auto expr : param_list) src.GetNode(expr).Visit();
    FindRef();
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
    // param list
    if (!param_list.empty()) {
        const std::string child_indent = indent + (is_last ? "  " : "| ");
        auto iter = param_list.cbegin();
        for (; iter != param_list.cend() - 1; ++iter) {
            src.GetNode(*iter).Dump(ostream, child_indent, false);
        }
        src.GetNode(*iter).Dump(ostream, child_indent, true);
    }
}

void CallExpr::FindRef() {
    const std::string name = GetIdentName();
    ASTLocation cur = parent;
    while (!has_ref) {
        ASTNode &cur_node = src.GetNode(cur);
        auto result = cur_node.FindIdent(name);
        if (result.first && ident > src.GetDecl(result.second).GetIdentLoc()) {
            has_ref = true;
            ref = result.second;
            continue;
        }
        if (cur_node.HasParent()) {
            cur = cur_node.GetParent();
            continue;
        }
        throw IdentRefNotFindException(GetIdentRange().Dump(), GetIdentName());
    }
}

/* class BinaryOperator */

void BinaryOperator::Link() {
    src.GetNode(LHS).SetParent(location);
    src.GetNode(RHS).SetParent(location);
}

void BinaryOperator::Visit() {
    src.GetNode(LHS).Visit();
    src.GetNode(RHS).Visit();
    CalculateValue();
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

void BinaryOperator::CheckOp() const {
    if (op_code < kAdd || kAssign < op_code) {
        std::stringstream dump;
        GetLHS().Dump(dump, "", true);
        dump << "\nop_code: " << std::to_string(op_code) << '\n';
        GetRHS().Dump(dump, "", true);
        throw InvalidOperatorException(dump.str());
    }
}

void BinaryOperator::CalculateValue() {
    is_const = false;
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

/* class UnaryOperator */

void UnaryOperator::Link() { src.GetNode(sub_expr).SetParent(location); }

void UnaryOperator::Visit() {
    src.GetNode(sub_expr).Visit();
    CalculateValue();
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

void UnaryOperator::CheckOp() const {
    if (op_code < kPlus || kNot < op_code) {
        std::stringstream dump;
        dump << "op_code: " << std::to_string(op_code) << '\n';
        GetSubExpr().Dump(dump, "", true);
        throw InvalidOperatorException(dump.str());
    }
}

void UnaryOperator::CalculateValue() {
    is_const = false;
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

/* class InitListExpr */

int InitListExpr::GetInitValue(const ASTManager &src,
                               const InitListExpr &target,
                               const std::vector<ASTLocation> &index) {
    if (target.is_filler) { return 0; }
    if (index.size() == 1) {
        return target.GetInitAt(src.GetExpr(index.front()).GetValue())
            .GetValue();
    }
    return GetInitValue(src,
                        target.GetInitAt(src.GetExpr(index.front()).GetValue())
                            .Cast<InitListExpr>(),
                        {index.begin() + 1, index.end()});
}

std::vector<int> InitListExpr::GetInitMap() const {
    if (is_filler) {
        int size = 1;
        for (auto dim : format) size *= dim;
        return {std::vector<int>(size, 0)};
    }

    std::vector<int> map;
    if (format.size() == 1) {
        for (auto expr : init_list) {
            map.emplace_back(src.GetExpr(expr).GetValue());
        }
    } else {
        for (auto expr : init_list) {
            auto sub_map = src.GetNode(expr).Cast<InitListExpr>().GetInitMap();
            map.insert(map.cend(), sub_map.cbegin(), sub_map.cend());
        }
    }
    return map;
}
std::vector<std::pair<bool, ASTLocation>> InitListExpr::GetInitMapExpr() const {
    if (is_filler) {
        int size = 1;
        for (auto dim : format) size *= dim;
        std::vector<std::pair<bool, ASTLocation>> result;
        while (size-- != 0) result.emplace_back(false, 0);
        return result;
    }

    std::vector<std::pair<bool, ASTLocation>> map;
    if (format.size() == 1) {
        for (auto expr : init_list) { map.emplace_back(true, expr); }
    } else {
        for (auto expr : init_list) {
            auto sub_map
                = src.GetNode(expr).Cast<InitListExpr>().GetInitMapExpr();
            map.insert(map.cend(), sub_map.cbegin(), sub_map.cend());
        }
    }
    return map;
}

std::string InitListExpr::TypeStr() const {
    std::string type_str = "int";
    for (auto len : format) type_str += '[' + std::to_string(len) + ']';
    return type_str;
}

void InitListExpr::Link() {
    for (auto expr : init_list) src.GetNode(expr).SetParent(location);
}

void InitListExpr::Visit() {
    for (auto expr : init_list) src.GetNode(expr).Visit();
    CalculateValue();
}

void InitListExpr::Dump(std::ostream &ostream,
                        const std::string &indent,
                        const bool is_last) const {
    DumpIndentAndBranch(ostream, indent, is_last);
    if (is_filler) {
        ostream << util::FormatTerminal("array_filler:", util::kFGBlue) << ' ';
    }
    DumpInfo(ostream, "ArrayInitList");
    // type
    ostream << ' '
            << util::FormatTerminal('\'' + TypeStr() + '\'', util::kFGGreen);
    // newline
    ostream << std::endl;
    // init list
    if (!init_list.empty()) {
        const std::string child_indent = indent + (is_last ? "  " : "| ");
        for (auto iter = init_list.cbegin(); iter != init_list.cend(); ++iter) {
            src.GetExpr(*iter).Dump(ostream, child_indent,
                                    *iter == init_list.back());
        }
    }
}

ASTLocation InitListExpr::Format(ASTManager &src,
                                 const SourceRange &range,
                                 const std::vector<ASTLocation> &list,
                                 const std::vector<int> &format) {
    // filler
    if (list.empty()) {
        return src.AddNode(new InitListExpr(src, range, list, format, true));
    }
    std::vector<ASTLocation> new_list;
    auto iter = list.begin();
    // for one dimension, padding missing 0
    if (format.size() == 1) {
        for (int i = 0; i < format.front(); ++i, ++iter) {
            if (iter < list.end()) {
                new_list.emplace_back(*iter);
            } else {
                new_list.emplace_back(
                    src.AddNode(new IntegerLiteral(src, {}, 0, true)));
            }
        }
    }
    // multiple dimension
    else {
        int sub_size = 1;
        for (auto iter = format.cbegin() + 1; iter != format.cend(); ++iter) {
            sub_size *= *iter;
        }
        for (int i = 0; i < format.front(); ++i, ++iter) {
            if (iter < list.end()) {
                if (src.GetNode(*iter).kind != kInitListExpr) {
                    auto last = iter + sub_size < list.end() ? iter + sub_size
                                                             : list.end();
                    new_list.emplace_back(Format(
                        src,
                        src.GetNode(*iter).GetRange()
                            + src.GetNode(*(last - 1)).GetRange(),
                        {iter, last}, {format.cbegin() + 1, format.cend()}));
                    iter += sub_size - 1;
                } else {
                    new_list.emplace_back(Format(
                        src, src.GetNode(*iter).GetRange(),
                        src.GetNode(*iter).Cast<InitListExpr>().GetInitList(),
                        {format.cbegin() + 1, format.cend()}));
                }
            } else {
                new_list.emplace_back(src.AddNode(new InitListExpr(
                    src, {}, {}, {format.cbegin() + 1, format.cend()}, true)));
            }
        }
    }
    return src.AddNode(new InitListExpr(src, range, new_list, format));
}

void InitListExpr::CalculateValue() {
    for (auto init : init_list) {
        if (!src.GetExpr(init).IsConst()) return;
    }
    is_const = true;
}

}  // namespace ast
