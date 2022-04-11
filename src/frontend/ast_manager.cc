#include "frontend/ast_manager.h"

#include "error.h"

namespace ast {

void ASTManager::RegisterIdent(const TokenLocation loc) {
    ident_table.emplace_back(loc);
}

void ASTManager::RegisterGlobalIdent(const std::string &name,
                                     const TokenLocation loc) {
    ident_table.emplace_back(loc);
    global_ident_table.emplace(name, loc);
}

TokenLocation ASTManager::FindRef(const TokenLocation loc) const {
    int indent_cur = src.GetIdentTokenIndent(loc);
    int indent_find;
    for (auto iter = ident_table.crbegin(); iter != ident_table.crend();
         ++iter) {
        if (indent_cur == IdentToken::kGlobal) return FindGlobalRef(loc);
        indent_find = src.GetIdentTokenIndent(*iter);
        if (indent_find > indent_cur) continue;
        if (indent_find < indent_cur) indent_cur = indent_find;
        // indent_cur == indent_find
        if (src.GetTokenText(loc) == src.GetTokenText(*iter)) return *iter;
    }
    throw IdentRefNotFindException(src.GetFileName(),
                                   src.GetTokenRange(loc).Dump(),
                                   src.GetTokenText(loc));
}

TokenLocation ASTManager::FindGlobalRef(const TokenLocation loc) const {
    auto ref = global_ident_table.find(src.GetTokenText(loc));
    if (ref == global_ident_table.end()) {
        throw IdentRefNotFindException(src.GetFileName(),
                                       src.GetTokenRange(loc).Dump(),
                                       src.GetTokenText(loc));
    }
    return ref->second;
}

ASTLocation ASTManager::AddDecl(Decl *decl, const TokenRange &context) {
    ASTNode_table.emplace_back(decl, nullptr);
    ASTNode_context.emplace(ASTNode_table.size() - 1, context);
    return ASTNode_table.size() - 1;
}

ASTLocation ASTManager::AddStmt(Stmt *stmt, const TokenRange &context) {
    ASTNode_table.emplace_back(nullptr, stmt);
    ASTNode_context.emplace(ASTNode_table.size() - 1, context);
    return ASTNode_table.size() - 1;
}

}  // namespace ast
