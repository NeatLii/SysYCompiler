#ifndef __sysycompiler_frontend_ast_manager_h__
#define __sysycompiler_frontend_ast_manager_h__

#include <memory>
#include <unordered_map>
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
    explicit ASTManager(SourceManager &raw,
                        const bool has_root = false,
                        const ASTLocation root = 0) noexcept
        : raw(raw), has_root(has_root), root(root) {}

    SourceManager &GetSourceManager() const { return raw; }

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
    SourceManager &raw;
    std::vector<std::unique_ptr<ASTNode>> node_table;

    bool has_root;
    ASTLocation root;
};

/*
 * Only TranslationUnit and CompoundStmt will store ident.
 * CompoundStmt also store FuncFParam.
 */

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
            ASTManager &src,
            const SourceRange &range = {},
            const bool has_location = false,
            const ASTLocation location = 0,
            const bool has_parent = false,
            const ASTLocation parent = 0)
        : kind(kind)
        , src(src)
        , range(range)
        , has_location(has_location)
        , location(location)
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

    template <typename T>
    T &Cast() const {
        return dynamic_cast<T &>(src.GetNode(location));
    }

    ASTManager &GetASTManager() const { return src; }
    SourceManager &GetSourceManager() const { return src.GetSourceManager(); }

    void SetRange(const SourceRange &range) { this->range = range; }
    const SourceRange &GetRange() const { return range; }

    bool HasLocation() const { return has_location; }
    void SetLocation(const ASTLocation location) {
        has_location = true;
        this->location = location;
    }
    ASTLocation GetLocation() const { return location; }

    bool HasParent() const { return has_parent; }
    void SetParent(const ASTLocation parent) {
        has_parent = true;
        this->parent = parent;
    }
    ASTLocation GetParent() const { return parent; }

    // Binding childs' parent ot myself.
    virtual void Link() = 0;

    // Traverse, in order to find reference and calculate const values.
    virtual void Visit() = 0;

    // ast-dump
    virtual void Dump(std::ostream &ostream,
                      const std::string &indent,
                      bool is_last) const = 0;

  protected:
    ASTManager &src;
    SourceRange range;

    bool has_location;
    ASTLocation location;
    bool has_parent;
    ASTLocation parent;

    static void DumpIndentAndBranch(std::ostream &ostream,
                                    const std::string &indent,
                                    bool is_last);
    void DumpInfo(std::ostream &ostream, const std::string &kind) const;
};

class TranslationUnit final : public ASTNode {
  public:
    explicit TranslationUnit(ASTManager &src) : ASTNode(kTranslationUnit, src) {
        BuiltIn();
    }

    void AddDecl(ASTLocation loc);
    const std::vector<ASTLocation> &GetDeclList() const { return decl_list; }
    std::vector<ASTLocation>::size_type GetDeclNum() const {
        return decl_list.size();
    }
    ASTLocation GetDeclAt(
        const std::vector<ASTLocation>::size_type index) const {
        return decl_list[index];
    }

    void Link() override {}

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    std::vector<ASTLocation> decl_list;

    void BuiltIn();
};

class Decl : public ASTNode {
  public:
    enum Type { kUndef, kVoid, kInt };

    Decl(const ASTNodeKind kind,
         ASTManager &src,
         const SourceRange &range,
         const TokenLocation ident,
         const Type type = kUndef)
        : ASTNode(kind, src, range), ident(ident), type(type) {}

    TokenLocation GetIdentLoc() const { return ident; }
    const Token &GetIdentToken() const {
        return src.GetSourceManager().GetToken(ident);
    }
    std::string GetIdentName() const {
        return src.GetSourceManager().GetTokenText(ident);
    }
    const SourceRange &GetIdentRange() const {
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
    VarDecl(ASTManager &src,
            const SourceRange &range,
            const TokenLocation ident,
            std::vector<ASTLocation> arr_dim_list = {},
            const bool is_const = false)
        : Decl(kVarDecl, src, range, ident)
        , has_init(false)
        , init(0)
        , arr_dim_list(std::move(arr_dim_list))
        , is_const(is_const) {}
    VarDecl(ASTManager &src,
            const SourceRange &range,
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
    ASTLocation GetInitLoc() const { return init; }
    const InitListExpr &GetInitList() const {
        return src.GetNode(init).Cast<InitListExpr>();
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

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool has_init;
    ASTLocation init;

    const std::vector<ASTLocation> arr_dim_list;
    bool is_const;
};

class ParamVarDecl final : public Decl {
  public:
    ParamVarDecl(ASTManager &src,
                 const SourceRange &range,
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

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool is_ptr;
    const std::vector<ASTLocation> arr_dim_list;
};

class FunctionDecl final : public Decl {
  public:
    FunctionDecl(ASTManager &src,
                 const SourceRange &range,
                 const TokenLocation ident,
                 std::vector<ASTLocation> param_list = {})
        : Decl(kFunctionDecl, src, range, ident)
        , param_list(std::move(param_list))
        , has_def(false)
        , def(0) {}
    FunctionDecl(ASTManager &src,
                 const SourceRange &range,
                 const TokenLocation ident,
                 const ASTLocation def,
                 std::vector<ASTLocation> param_list = {})
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
    void SetDef(ASTLocation def);
    const CompoundStmt &GetDef() const {
        return src.GetNode(def).Cast<CompoundStmt>();
    }
    ASTLocation GetDefLoc() const { return def; }

    std::string TypeStr() const override;

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> param_list;
    bool has_def;
    ASTLocation def;
};

class Stmt : public ASTNode {
  public:
    Stmt(const ASTNodeKind kind, ASTManager &src, const SourceRange &range)
        : ASTNode(kind, src, range) {}
};

class CompoundStmt final : public Stmt {
  public:
    CompoundStmt(ASTManager &src,
                 const SourceRange &range,
                 std::vector<ASTLocation> stmt_list = {})
        : Stmt(kCompoundStmt, src, range), stmt_list(std::move(stmt_list)) {}

    const std::vector<ASTLocation> &GetStmtList() const { return stmt_list; }
    std::vector<ASTLocation>::size_type GetStmtNum() const {
        return stmt_list.size();
    }
    const Stmt &GetStmtAt(std::vector<ASTLocation>::size_type index) const {
        return src.GetStmt(stmt_list[index]).Cast<Stmt>();
    }

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> stmt_list;
};

class DeclStmt final : public Stmt {
  public:
    DeclStmt(ASTManager &src,
             const SourceRange &range,
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

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const std::vector<ASTLocation> decl_list;
};

class NullStmt final : public Stmt {
  public:
    explicit NullStmt(ASTManager &src, const SourceRange &range)
        : Stmt(kNullStmt, src, range) {}

    void Link() override {}

    void Visit() override {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class IfStmt final : public Stmt {
  public:
    IfStmt(ASTManager &src,
           const SourceRange &range,
           const ASTLocation cond,
           const ASTLocation then_stmt)
        : Stmt(kIfStmt, src, range)
        , cond(cond)
        , then_stmt(then_stmt)
        , has_else(false)
        , else_stmt(0) {}
    IfStmt(ASTManager &src,
           const SourceRange &range,
           const ASTLocation cond,
           const ASTLocation then_stmt,
           const ASTLocation else_stmt)
        : Stmt(kIfStmt, src, range)
        , cond(cond)
        , then_stmt(then_stmt)
        , has_else(true)
        , else_stmt(else_stmt) {}

    const Expr &GetCond() const { return src.GetExpr(cond); }
    ASTLocation GetCondLoc() const { return cond; }

    const Stmt &GetThen() const { return src.GetStmt(then_stmt); }
    ASTLocation GetThenLoc() const { return then_stmt; }

    bool HasElse() const { return has_else; }

    const Stmt &GetElse() const { return src.GetStmt(else_stmt); }
    ASTLocation GetElseLoc() const { return else_stmt; }

    void Link() override;

    void Visit() override;

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
    WhileStmt(ASTManager &src,
              const SourceRange &range,
              const ASTLocation cond,
              const ASTLocation body)
        : Stmt(kWhileStmt, src, range), cond(cond), body(body) {}

    const Expr &GetCond() const { return src.GetExpr(cond); }
    ASTLocation GetCondLoc() const { return cond; }

    const Stmt &GetBody() const { return src.GetStmt(body); }
    ASTLocation GetBodyLoc() const { return body; }

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation cond;
    const ASTLocation body;
};

class ContinueStmt final : public Stmt {
  public:
    explicit ContinueStmt(ASTManager &src, const SourceRange &range)
        : Stmt(kContinueStmt, src, range) {}

    void Link() override {}

    void Visit() override {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class BreakStmt final : public Stmt {
  public:
    explicit BreakStmt(ASTManager &src, const SourceRange &range)
        : Stmt(kBreakStmt, src, range) {}

    void Link() override {}

    void Visit() override {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;
};

class ReturnStmt final : public Stmt {
  public:
    ReturnStmt(ASTManager &src, const SourceRange &range)
        : Stmt(kReturnStmt, src, range), has_expr(false), expr(0) {}
    ReturnStmt(ASTManager &src,
               const SourceRange &range,
               const ASTLocation expr)
        : Stmt(kReturnStmt, src, range), has_expr(true), expr(expr) {}

    bool HasExpr() const { return has_expr; }

    const Expr &GetExpr() const { return src.GetExpr(expr); }
    ASTLocation GetExprLoc() const { return expr; }

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool has_expr;
    const ASTLocation expr;
};

class Expr : public Stmt {
  public:
    Expr(const ASTNodeKind kind, ASTManager &src, const SourceRange &range)
        : Stmt(kind, src, range), is_const(false), value(0) {}
    Expr(const ASTNodeKind kind,
         ASTManager &src,
         const SourceRange &range,
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
    IntegerLiteral(ASTManager &src,
                   const SourceRange &range,
                   const int value,
                   const bool is_filler = false)
        : Expr(kIntegerLiteral, src, range, value), is_filler(is_filler) {}

    void Link() override {}

    void Visit() override {}

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const bool is_filler;
};

class ParenExpr final : public Expr {
  public:
    ParenExpr(ASTManager &src,
              const SourceRange &range,
              const ASTLocation sub_expr)
        : Expr(kParenExpr, src, range), sub_expr(sub_expr) {}

    const Expr &GetSubExpr() const { return src.GetExpr(sub_expr); }
    ASTLocation GetSubExprLoc() const { return sub_expr; }

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation sub_expr;
};

class DeclRefExpr final : public Expr {
  public:
    DeclRefExpr(ASTManager &src,
                const SourceRange &range,
                const TokenLocation ident,
                std::vector<ASTLocation> arr_dim_list = {},
                const bool has_ref = false,
                const ASTLocation ref = 0)
        : Expr(kDeclRefExpr, src, range)
        , ident(ident)
        , arr_dim_list(std::move(arr_dim_list))
        , has_ref(has_ref)
        , ref(ref) {}

    TokenLocation GetIdentLoc() const { return ident; }
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
    const Decl &GetRef() const { return src.GetDecl(ref); }

    bool ResultIsArr() const;

    std::string TypeStr() const;

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const TokenLocation ident;
    const std::vector<ASTLocation> arr_dim_list;

    bool has_ref;
    ASTLocation ref;

    void FindRef();
    void CalculateValue();
};

class CallExpr final : public Expr {
  public:
    CallExpr(ASTManager &src,
             const SourceRange &range,
             const TokenLocation ident,
             std::vector<ASTLocation> param_list = {},
             const bool has_ref = false,
             const ASTLocation ref = 0)
        : Expr(kCallExpr, src, range)
        , ident(ident)
        , param_list(std::move(param_list))
        , has_ref(has_ref)
        , ref(ref) {}

    TokenLocation GetIdentLoc() const { return ident; }
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
    const FunctionDecl &GetRef() const {
        return src.GetNode(ref).Cast<const FunctionDecl &>();
    }

    std::string TypeStr() const;

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const TokenLocation ident;
    const std::vector<ASTLocation> param_list;

    bool has_ref;
    ASTLocation ref;

    void FindRef();
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

    BinaryOperator(ASTManager &src,
                   const SourceRange &range,
                   const BinaryOpKind op_code,
                   const ASTLocation LHS,
                   const ASTLocation RHS)
        : Expr(kBinaryOperator, src, range)
        , op_code(op_code)
        , LHS(LHS)
        , RHS(RHS) {
        CheckOp();
    }

    const Expr &GetLHS() const { return src.GetExpr(LHS); }
    ASTLocation GetLHSLoc() const { return LHS; }

    const Expr &GetRHS() const { return src.GetExpr(RHS); }
    ASTLocation GetRHSLoc() const { return RHS; }

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation LHS;
    const ASTLocation RHS;

    void CheckOp() const;
    void CalculateValue();
};

class UnaryOperator final : public Expr {
  public:
    enum UnaryOpKind { kPlus, kMinus, kNot };
    const UnaryOpKind op_code;

    UnaryOperator(ASTManager &src,
                  const SourceRange &range,
                  const UnaryOpKind op_code,
                  const ASTLocation sub_expr)
        : Expr(kUnaryOperator, src, range)
        , op_code(op_code)
        , sub_expr(sub_expr) {
        CheckOp();
    }

    ASTLocation GetSubExprLoc() const { return sub_expr; }
    const Expr &GetSubExpr() const { return src.GetExpr(sub_expr); }

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

  private:
    const ASTLocation sub_expr;

    void CheckOp() const;
    void CalculateValue();
};

class InitListExpr final : public Expr {
  public:
    InitListExpr(ASTManager &src,
                 const SourceRange &range,
                 std::vector<ASTLocation> init_list = {},
                 std::vector<int> format = {},
                 const bool is_filler = false)
        : Expr(kInitListExpr, src, range)
        , init_list(std::move(init_list))
        , format(std::move(format))
        , is_filler(is_filler) {}

    const std::vector<ASTLocation> &GetInitList() const { return init_list; }
    std::vector<ASTLocation>::size_type GetInitNum() const {
        return init_list.size();
    }
    const Expr &GetInitAt(std::vector<ASTLocation>::size_type index) const {
        return src.GetExpr(init_list[index]);
    }

    static int GetInitValue(const ASTManager &src,
                            const InitListExpr &target,
                            const std::vector<ASTLocation> &index);

    std::vector<int> GetInitMap() const;
    std::vector<std::pair<bool, ASTLocation>> GetInitMapExpr() const;

    void SetFormat(const std::vector<int> &format) { this->format = format; }
    const std::vector<int> &GetFormat() const { return format; }

    std::string TypeStr() const;

    void Link() override;

    void Visit() override;

    void Dump(std::ostream &ostream,
              const std::string &indent,
              bool is_last) const override;

    static ASTLocation Format(ASTManager &src,
                              const SourceRange &range,
                              const std::vector<ASTLocation> &list,
                              const std::vector<int> &format);

  private:
    std::vector<ASTLocation> init_list;

    std::vector<int> format;
    const bool is_filler;

    void CalculateValue();
};

}  // namespace ast

#endif
