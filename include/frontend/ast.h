#ifndef __ast_h__
#define __ast_h__

#include <ostream>

#include "frontend/source_manager.h"

namespace ast {

class ASTManager;

// basic types
enum Type { kUndef, kVoid, kInt };

class Decl {
  public:
    Decl(const SourceManager &src,
         const TokenLocation ident,
         const Type type,
         const ASTManager &def)
        : src(src), ident(ident), type(type), def(def) {}
    virtual ~Decl() = default;
    Decl(const Decl &) = delete;
    Decl &operator=(const Decl &) = delete;
    Decl(Decl &&) = delete;
    Decl &operator=(Decl &&) = delete;

    std::string GetName() const { return src.GetTokenText(ident); }
    Type GetType() const { return type; }

    virtual std::string Dump(std::ostream &ostream,
                             const std::string &indent) const = 0;

  protected:
    const SourceManager &src;
    const TokenLocation ident;
    const Type type;
    const ASTManager &def;
};

class Stmt {
  public:
    explicit Stmt(const ASTManager &def) : def(def) {}
    virtual ~Stmt() = default;
    Stmt(const Stmt &) = delete;
    Stmt &operator=(const Stmt &) = delete;
    Stmt(Stmt &&) = delete;
    Stmt &operator=(Stmt &&) = delete;

    virtual std::string Dump(std::ostream &ostream,
                             const std::string &indent) const = 0;

  protected:
    const ASTManager &def;
};

}  // namespace ast

#endif
