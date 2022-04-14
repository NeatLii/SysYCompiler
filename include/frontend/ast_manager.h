#ifndef __ast_manager_h__
#define __ast_manager_h__

#include <error.h>

#include <memory>
#include <ostream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "frontend/source_manager.h"

namespace ast {

/* declarations */

class ASTManager;
class IdentTable;

class ASTNode;
// index in ASTNode table
using ASTLocation = std::vector<ASTNode>::size_type;

class TranslationUnit;

class Decl;
class VarDecl;
class ParamVarDecl;
class FunctionDecl;

class Stmt;
class CompoundStmt;
class DeclStmt;
class NullStmt;
class IfStmt;
class WhileStmt;
class ContinueStmt;
class BreakStmt;
class ReturnStmt;

class Expr;
class ConstExpr;
class IntegerLiteral;
class ParenExpr;
class DeclRefExpr;
class CallExpr;
class BinaryOperator;
class UnaryOperator;
class InitListExpr;

/* definitions */

class ASTManager {
  public:
    explicit ASTManager(const SourceManager &raw,
                        const bool has_root = false,
                        const ASTLocation root = 0)
        : raw(raw), has_root(has_root), root(root) {}

    const SourceManager &GetSourceManager() const { return raw; }

    ASTLocation AddNode(ASTNode *node);
    ASTLocation AddNode(std::unique_ptr<ASTNode> &node);
    ASTNode &GetNode(const ASTLocation loc) const { return *node_table[loc]; }

    bool HasRoot() const { return has_root; }
    void SetRoot(ASTLocation root);
    void SetRoot(ASTNode *root);
    void SetRoot(std::unique_ptr<ASTNode> &root);

    TranslationUnit &GetRoot() const;
    Decl &GetDecl(ASTLocation loc) const;
    Stmt &GetStmt(ASTLocation loc) const;
    Expr &GetExpr(ASTLocation loc) const;

    void Dump(std::ostream &ostream) const;

  private:
    const SourceManager &raw;
    std::vector<std::unique_ptr<ASTNode>> node_table;

    bool has_root;
    ASTLocation root;
};

class IdentTable {
  public:
    IdentTable() = default;

    void AddIdent(const std::string &name, const ASTLocation loc) {
        ident_table.emplace(name, loc);
    }
    std::pair<bool, ASTLocation> FindIdent(const std::string &name) const;

    const std::unordered_map<std::string, ASTLocation> &GetIdentTable() const {
        return ident_table;
    }
    std::unordered_map<std::string, ASTLocation>::size_type GetIdentNum()
        const {
        return ident_table.size();
    }

    void DumpIdentTable(std::ostream &ostream) const;

  protected:
    std::unordered_map<std::string, ASTLocation> ident_table;
};

class ASTNode : public IdentTable {
  public:
    enum ASTNodeKind {
        kASTNode,

        kTranslationUnit,

        kDecl,
        kVarDecl,
        kParamVarDecl,
        kFunctionDecl,

        kStmt,
        kCompoundStmt,
        kDeclStmt,
        kNullStmt,
        kIfStmt,
        kWhileStmt,
        kContinueStmt,
        kBreakStmt,
        kReturnStmt,

        kExpr,
        kConstExpr,
        kIntegerLiteral,
        kParenExpr,
        kDeclRefExpr,
        kCallExpr,
        kBinaryOperator,
        kUnaryOperator,
        kInitListExpr,
    };
    const ASTNodeKind kind;

    ASTNode(const ASTNodeKind kind,
            const ASTManager &src,
            const SourceRange range = {0, 0, 0, 0},
            const bool has_loc = false,
            const ASTLocation loc = 0,
            const bool has_parent = false,
            const ASTLocation parent = 0)
        : kind(kind)
        , src(src)
        , range(range)
        , has_loc(has_loc)
        , loc(loc)
        , has_parent(has_parent)
        , parent(parent) {}
    virtual ~ASTNode() = default;
    ASTNode(const ASTNode &) = delete;
    ASTNode &operator=(const ASTNode &) = delete;
    ASTNode(ASTNode &&) = delete;
    ASTNode &operator=(ASTNode &&) = delete;

    bool IsDecl() const {
        return (kDecl <= kind && kind <= kFunctionDecl)
               || kind == kTranslationUnit;
    }
    bool IsStmt() const { return kStmt <= kind && kind <= kInitListExpr; }
    bool IsExpr() const { return kExpr <= kind && kind <= kInitListExpr; }

    const ASTManager &GetASTManager() const { return src; }

    void SetRange(const SourceRange range) { this->range = range; }
    // void SetBegin(const TokenLocation loc) { range.begin = loc; }
    // void SetEnd(const TokenLocation loc) { range.end = loc; }
    SourceRange GetRange() const { return range; }
    // TokenLocation GetBegin() const { return range.begin; }
    // TokenLocation GetEnd() const { return range.end; }

    bool HasLocation() const { return has_loc; }
    void SetLocation(const ASTLocation loc) {
        has_loc = true;
        this->loc = loc;
    }
    ASTLocation GetLocation() const { return has_loc ? loc : 0; }

    bool HasParent() const { return has_parent; }
    void SetParent(const ASTLocation parent) {
        has_parent = true;
        this->parent = parent;
    }
    ASTLocation GetParent() const { return has_parent ? parent : 0; }

    virtual void Dump(std::ostream &ostream,
                      const std::string &indent,
                      bool is_last) const = 0;

    template <typename T>
    T &Cast() const {
        return dynamic_cast<T &>(src.GetNode(loc));
    }

  protected:
    const ASTManager &src;
    SourceRange range;

    bool has_loc;
    ASTLocation loc;
    bool has_parent;
    ASTLocation parent;

    static void DumpIndentAndBranch(std::ostream &ostream,
                                    const std::string &indent,
                                    bool is_last);
    void DumpInfo(std::ostream &ostream, const std::string &kind) const;
};

class TranslationUnit final : public ASTNode {
  public:
    explicit TranslationUnit(const ASTManager &src)
        : ASTNode(kTranslationUnit, src) {}

    void AddDecl(ASTLocation loc);
    const std::vector<ASTLocation> &GetDeclList() const { return decl_list; }
    std::vector<ASTLocation>::size_type GetDeclNum() const {
        return decl_list.size();
    }
    ASTLocation GetDeclAt(const std::vector<ASTLocation>::size_type index) {
        return decl_list[index];
    }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    std::vector<ASTLocation> decl_list;
};

class Decl : public ASTNode {
  public:
    enum Type { kUndef, kVoid, kInt };

    Decl(const ASTNodeKind kind,
         const ASTManager &src,
         const SourceRange range,
         const TokenLocation ident,
         const Type type = kUndef)
        : ASTNode(kind, src, range), ident(ident), type(type) {}

    const Token &GetIdentToken() const {
        return src.GetSourceManager().GetToken(ident);
    }
    std::string GetIdentName() const {
        return src.GetSourceManager().GetTokenText(ident);
    }
    SourceRange GetIdentRange() const {
        return src.GetSourceManager().GetTokenRange(ident);
    }

    void SetType(const Type type) { this->type = type; }
    Type GetType() const { return type; }
    virtual std::string TypeStr() const = 0;

  protected:
    const TokenLocation ident;
    Type type;
};

class VarDecl final : public Decl {
  public:
    VarDecl(const ASTManager &src,
            const SourceRange range,
            const TokenLocation ident,
            std::vector<ASTLocation> arr_dim_list = {},
            const bool is_const = false)
        : Decl(kVarDecl, src, range, ident)
        , has_init(false)
        , init(0)
        , arr_dim_list(std::move(arr_dim_list))
        , is_const(is_const) {}
    VarDecl(const ASTManager &src,
            const SourceRange range,
            const TokenLocation ident,
            const ASTLocation init,
            std::vector<ASTLocation> arr_dim_list = {},
            const bool is_const = false)
        : Decl(kVarDecl, src, range, ident)
        , has_init(true)
        , init(init)
        , arr_dim_list(std::move(arr_dim_list))
        , is_const(is_const) {}

    bool HasInit() const { return has_init; }
    const Expr &GetInit() const { return src.GetExpr(init); }
    const InitListExpr &GetInitList() const {
        return src.GetNode(loc).Cast<InitListExpr>();
    }

    bool IsArray() const { return !arr_dim_list.empty(); }
    const std::vector<ASTLocation> &GetArrDimList() const {
        return arr_dim_list;
    }
    std::vector<ASTLocation>::size_type GetArrDimNum() const {
        return arr_dim_list.size();
    }
    const Expr &GetArrDimAt(
        const std::vector<ASTLocation>::size_type index) const {
        return src.GetExpr(arr_dim_list[index]);
    }

    void SetConst(const bool is_const) { this->is_const = is_const; }
    bool IsConst() const { return is_const; }

    std::string TypeStr() const override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool has_init;
    const ASTLocation init;

    const std::vector<ASTLocation> arr_dim_list;
    bool is_const;
};

class ParamVarDecl final : public Decl {
  public:
    ParamVarDecl(const ASTManager &src,
                 const SourceRange range,
                 const TokenLocation ident,
                 const bool is_ptr = false,
                 std::vector<ASTLocation> arr_dim_list = {})
        : Decl(kParamVarDecl, src, range, ident)
        , is_ptr(is_ptr)
        , arr_dim_list(std::move(arr_dim_list)) {}

    bool IsPtr() const { return is_ptr; }
    bool IsArrayPtr() const { return is_ptr && !arr_dim_list.empty(); }

    const std::vector<ASTLocation> &GetArrDimList() const {
        return arr_dim_list;
    }
    std::vector<ASTLocation>::size_type GetArrDimNum() const {
        return arr_dim_list.size();
    }
    const Expr &GetArrDimAt(
        const std::vector<ASTLocation>::size_type index) const {
        return src.GetExpr(arr_dim_list[index]);
    }

    std::string TypeStr() const override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool is_ptr;
    const std::vector<ASTLocation> arr_dim_list;
};

class FunctionDecl final : public Decl {
  public:
    FunctionDecl(const ASTManager &src,
                 const SourceRange range,
                 const TokenLocation ident,
                 std::vector<ASTLocation> param_list = {})
        : Decl(kFunctionDecl, src, range, ident)
        , param_list(std::move(param_list))
        , has_def(false)
        , def(0) {}
    FunctionDecl(const ASTManager &src,
                 const SourceRange range,
                 const TokenLocation ident,
                 std::vector<ASTLocation> param_list,
                 const ASTLocation def)
        : Decl(kFunctionDecl, src, range, ident)
        , param_list(std::move(param_list))
        , has_def(true)
        , def(def) {}

    const std::vector<ASTLocation> &GetParamList() const { return param_list; }
    std::vector<ASTLocation>::size_type GetParamNum() const {
        return param_list.size();
    }
    const ParamVarDecl &GetParamAt(
        std::vector<ASTLocation>::size_type index) const {
        return src.GetNode(param_list[index]).Cast<ParamVarDecl>();
    }

    bool HasDef() const { return has_def; }
    const CompoundStmt &GetDef() const {
        return src.GetNode(def).Cast<CompoundStmt>();
    }

    std::string TypeStr() const override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> param_list;
    const bool has_def;
    const ASTLocation def;
};

class Stmt : public ASTNode {
  public:
    Stmt(const ASTNodeKind kind, const ASTManager &src, const SourceRange range)
        : ASTNode(kind, src, range) {}
};

class CompoundStmt final : public Stmt {
  public:
    CompoundStmt(const ASTManager &src,
                 const SourceRange range,
                 std::vector<ASTLocation> stmt_list = {})
        : Stmt(kCompoundStmt, src, range), stmt_list(std::move(stmt_list)) {}

    const std::vector<ASTLocation> &GetStmtList() const { return stmt_list; }
    std::vector<ASTLocation>::size_type GetStmtNum() const {
        return stmt_list.size();
    }
    const Stmt &GetStmtAt(std::vector<ASTLocation>::size_type index) const {
        return src.GetStmt(stmt_list[index]).Cast<Stmt>();
    }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> stmt_list;
};

class DeclStmt final : public Stmt {
  public:
    DeclStmt(const ASTManager &src,
             const SourceRange range,
             std::vector<ASTLocation> decl_list)
        : Stmt(kDeclStmt, src, range), decl_list(std::move(decl_list)) {}

    const std::vector<ASTLocation> &GetDeclList() const { return decl_list; }
    std::vector<ASTLocation>::size_type GetDeclNum() const {
        return decl_list.size();
    }
    const VarDecl &GetDeclAt(
        const std::vector<ASTLocation>::size_type index) const {
        return src.GetNode(decl_list[index]).Cast<VarDecl>();
    }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> decl_list;
};

class NullStmt final : public Stmt {
  public:
    explicit NullStmt(const ASTManager &src, const SourceRange range)
        : Stmt(kNullStmt, src, range) {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class IfStmt final : public Stmt {
  public:
    IfStmt(const ASTManager &src,
           const SourceRange range,
           const ASTLocation cond,
           const ASTLocation then_stmt)
        : Stmt(kIfStmt, src, range)
        , cond(cond)
        , then_stmt(then_stmt)
        , has_else(false)
        , else_stmt(0) {}
    IfStmt(const ASTManager &src,
           const SourceRange range,
           const ASTLocation cond,
           const ASTLocation then_stmt,
           const ASTLocation else_stmt)
        : Stmt(kIfStmt, src, range)
        , cond(cond)
        , then_stmt(then_stmt)
        , has_else(true)
        , else_stmt(else_stmt) {}

    const Expr &GetCond() const { return src.GetExpr(cond); }
    const Stmt &GetThen() const { return src.GetStmt(then_stmt); }
    const Stmt &GetElse() const { return src.GetStmt(else_stmt); }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation cond;
    const ASTLocation then_stmt;
    const bool has_else;
    const ASTLocation else_stmt;
};

class WhileStmt final : public Stmt {
  public:
    WhileStmt(const ASTManager &src,
              const SourceRange range,
              const ASTLocation cond,
              const ASTLocation body)
        : Stmt(kWhileStmt, src, range), cond(cond), body(body) {}

    const Expr &GetCond() const { return src.GetExpr(cond); }
    const Stmt &GetBody() const { return src.GetStmt(body); }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation cond;
    const ASTLocation body;
};

class ContinueStmt final : public Stmt {
  public:
    explicit ContinueStmt(const ASTManager &src, const SourceRange range)
        : Stmt(kContinueStmt, src, range) {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class BreakStmt final : public Stmt {
  public:
    explicit BreakStmt(const ASTManager &src, const SourceRange range)
        : Stmt(kBreakStmt, src, range) {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class ReturnStmt final : public Stmt {
  public:
    ReturnStmt(const ASTManager &src, const SourceRange range)
        : Stmt(kReturnStmt, src, range), has_expr(false), expr(0) {}
    ReturnStmt(const ASTManager &src,
               const SourceRange range,
               const ASTLocation expr)
        : Stmt(kReturnStmt, src, range), has_expr(true), expr(expr) {}

    bool HasExpr() const { return has_expr; }

    const Expr &GetExpr() const { return src.GetExpr(expr); }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool has_expr;
    const ASTLocation expr;
};

class Expr : public Stmt {
  public:
    Expr(const ASTNodeKind kind, const ASTManager &src, const SourceRange range)
        : Stmt(kind, src, range), is_const(false), value(0) {}
    Expr(const ASTNodeKind kind,
         const ASTManager &src,
         const SourceRange range,
         const int value)
        : Stmt(kind, src, range), is_const(true), value(value) {}

    bool IsConst() const { return is_const; }

    void SetValue(const int value) {
        is_const = true;
        this->value = value;
    }
    int GetValue() const { return value; }

  protected:
    bool is_const;
    int value;

    void DumpConstExpr(std::ostream &ostream) const;
};

class IntegerLiteral final : public Expr {
  public:
    IntegerLiteral(const ASTManager &src, SourceRange range, const int value)
        : Expr(kIntegerLiteral, src, range, value) {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class ParenExpr final : public Expr {
  public:
    ParenExpr(const ASTManager &src, SourceRange range, ASTLocation sub_expr);

    const Expr &GetSubExpr() const { return src.GetExpr(sub_expr); }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation sub_expr;
};

class DeclRefExpr final : public Expr {
  public:
    DeclRefExpr(const ASTManager &src,
                SourceRange range,
                TokenLocation ident,
                std::vector<ASTLocation> arr_dim_list = {},
                bool has_ref = false,
                ASTLocation ref = 0);

    const Token &GetIdentToken() const {
        return src.GetSourceManager().GetToken(ident);
    }
    std::string GetIdentName() const {
        return src.GetSourceManager().GetTokenText(ident);
    }
    SourceRange GetIdentRange() const {
        return src.GetSourceManager().GetTokenRange(ident);
    }

    bool IsArray() const { return !arr_dim_list.empty(); }
    const std::vector<ASTLocation> &GetArrDimList() const {
        return arr_dim_list;
    }
    std::vector<ASTLocation>::size_type GetArrDimNum() const {
        return arr_dim_list.size();
    }
    const Expr &GetArrDimAt(
        const std::vector<ASTLocation>::size_type index) const {
        return src.GetExpr(arr_dim_list[index]);
    }

    bool HasRef() const { return has_ref; }
    void SetRef(ASTLocation ref);
    const Decl &GetRef() const { return src.GetDecl(ref); }

    std::string TypeStr() const;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const TokenLocation ident;
    const std::vector<ASTLocation> arr_dim_list;

    bool has_ref;
    ASTLocation ref;
};

class CallExpr final : public Expr {
  public:
    CallExpr(const ASTManager &src,
             const SourceRange range,
             const TokenLocation ident,
             std::vector<ASTLocation> param_list = {},
             const bool has_ref = false,
             const ASTLocation ref = 0)
        : Expr(kCallExpr, src, range)
        , ident(ident)
        , param_list(std::move(param_list))
        , has_ref(has_ref)
        , ref(ref) {}

    const Token &GetIdentToken() const {
        return src.GetSourceManager().GetToken(ident);
    }
    std::string GetIdentName() const {
        return src.GetSourceManager().GetTokenText(ident);
    }
    SourceRange GetIdentRange() const {
        return src.GetSourceManager().GetTokenRange(ident);
    }

    const std::vector<ASTLocation> &GetParamList() const { return param_list; }
    std::vector<ASTLocation>::size_type GetParamNum() const {
        return param_list.size();
    }
    const Expr &GetParamAt(std::vector<ASTLocation>::size_type index) const {
        return src.GetExpr(param_list[index]);
    }

    bool HasRef() const { return has_ref; }
    void SetRef(const ASTLocation ref) {
        has_ref = true;
        this->ref = ref;
    }
    const FunctionDecl &GetRef() const {
        return src.GetNode(ref).Cast<const FunctionDecl &>();
    }

    std::string TypeStr() const;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const TokenLocation ident;
    const std::vector<ASTLocation> param_list;

    bool has_ref;
    ASTLocation ref;
};

class BinaryOperator final : public Expr {
  public:
    enum BinaryOpKind {
        kAdd,
        kSub,
        kMul,
        kDiv,
        kRem,

        kOr,
        kAnd,

        kEQ,
        kNE,
        kLT,
        kLE,
        kGT,
        kGE,

        kAssign
    };
    const BinaryOpKind op_code;

    BinaryOperator(const ASTManager &src,
                   SourceRange range,
                   BinaryOpKind op_code,
                   ASTLocation LHS,
                   ASTLocation RHS);

    const Expr &GetLHS() const { return src.GetExpr(LHS); }
    const Expr &GetRHS() const { return src.GetExpr(RHS); }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation LHS;
    const ASTLocation RHS;
};

class UnaryOperator final : public Expr {
  public:
    enum UnaryOpKind { kPlus, kMinus, kNot };
    const UnaryOpKind op_code;

    UnaryOperator(const ASTManager &src,
                  SourceRange range,
                  UnaryOpKind op_code,
                  ASTLocation sub_expr);

    const Expr &GetSubExpr() const { return src.GetExpr(sub_expr); }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation sub_expr;
};

class InitListExpr final : public Expr {
  public:
    InitListExpr(const ASTManager &src, std::vector<ASTLocation> list);

    const std::vector<ASTLocation> &GetInitList() const { return init_list; }
    std::vector<ASTLocation>::size_type GetInitNum() const {
        return init_list.size();
    }
    const Expr &GetInitAt(std::vector<ASTLocation>::size_type index) const {
        return src.GetExpr(init_list[index]);
    }

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> init_list;
};

}  // namespace ast

#endif
