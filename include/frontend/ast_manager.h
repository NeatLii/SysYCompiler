#ifndef __ast_manager_h__
#define __ast_manager_h__

#include <memory>
#include <unordered_map>

#include "frontend/ast.h"
#include "frontend/source_manager.h"

namespace ast {

class ASTNode final {
  public:
    ASTNode(Decl *decl, Stmt *stmt) : decl(decl), stmt(stmt) {}
    const Decl &GetDecl() const { return *decl; }
    const Stmt &GetStmt() const { return *stmt; }

  private:
    std::unique_ptr<Decl> decl;
    std::unique_ptr<Stmt> stmt;
};

// index in ASTNode table
using ASTLocation = std::vector<ASTNode>::size_type;
// range in token table
using TokenRange = struct {
    const TokenLocation begin;
    const TokenLocation end;
};

// manage ident table and ASTNode table
class ASTManager final {
  public:
    explicit ASTManager(const SourceManager &src) : src(src) {}

    void RegisterIdent(TokenLocation loc);
    void RegisterGlobalIdent(const std::string &name, TokenLocation loc);
    TokenLocation FindRef(TokenLocation loc) const;
    TokenLocation FindGlobalRef(TokenLocation loc) const;

    ASTLocation AddDecl(Decl *decl, const TokenRange &context);
    ASTLocation AddStmt(Stmt *stmt, const TokenRange &context);
    const Decl &GetDecl(ASTLocation loc) const {
        return ASTNode_table[loc].GetDecl();
    }
    const Stmt &GetStmt(ASTLocation loc) const {
        return ASTNode_table[loc].GetStmt();
    }

  private:
    const SourceManager &src;
    std::vector<TokenLocation> ident_table;
    std::unordered_map<std::string, TokenLocation> global_ident_table;
    std::vector<ASTNode> ASTNode_table;
    std::unordered_map<ASTLocation, TokenRange> ASTNode_context;
};
}  // namespace ast

#endif
