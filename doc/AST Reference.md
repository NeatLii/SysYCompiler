# Source Manager

保存源文件的信息、所有 token 的内容和位置。

```c++
struct SourceRange {
    int begin_line, begin_column;
    int end_line, end_column;
};

struct Token {
    std::string text;
    SourceRange range;
};

// used to count identifier scope
struct IdentToken : public Token {
    int indent;
};

// index in token table
using TokenLocation = std::vector<Token>::size_type;

class SourceMageger{
    std::string file_name;
	std::vector<Token> token_table;
}
```



# AST Manager

保存符号表、所有 AST 结点及其上下文。

```c++
class ASTNode {
    std::unique_ptr<Decl> decl;
    std::unique_ptr<Stmt> stmt;
}

// index in ASTNode table
using ASTLocation = std::vector<ASTNode>::size_type;
// range in token table
using TokenRange = struct {
    const TokenLocation begin;
    const TokenLocation end;
};

class ASTManager{
	const SourceManager &src;
    std::vector<TokenLocation> ident_table;
    std::unordered_map<std::string, TokenLocation> global_ident_table;
    std::vector<ASTNode> ASTNode_table;
    std::unordered_map<ASTLocation, TokenRange> ASTNode_context;
}
```



# AST

## Type

```c++
// basic types
enum Type { kUndef, kVoid, kInt };
```



## Decl (virtual)

- `SourceManager &src`
- `SourceLocation ident`：指向 `token_table` 中存放标识符的位置
- `Type type`
- `ASTManager &def`

### VarDecl

- `bool const_decl`
- `std::vector<int> array_dim`
- `ASTLocation init_expr`：`Expr` 类型

### FunctionDecl

- `std::vector<ASTLocation> params`：`ParamVarDecl` 类型
- `ASTLocation body`：`CompoundStmt` 类型

### ParamVarDecl

- `bool ptr_decl`
- `std::vector<int> array_dim`



## Stmt (virtual)

- `ASTManager &def`

### CompoundStmt

- `std::vector<ASTLocation> stmts`：`Stmt` 类型

### DeclStmt

- `ASTLocation decl`：`VarDecl` 类型

### NullStmt

### IfStmt

- `ASTLocation cond`：`Expr` 类型
- `ASTLocation if_true`：`Stmt` 类型
- `ASTLocation if_false`：`Stmt` 类型

### WhileStmt

- `ASTLocation cond`：`Expr` 类型
- `ASTLocation body`：`Stmt` 类型

### ContinueStmt

### BreakStmt

### ReturnStmt

- `ASTLocation expr`：`Expr` 类型

### Expr (virtual)

##### IntegerLiteral

- `int value`

##### ParenExpr

- `ASTLocation expr`：`Expr` 类型

##### DeclRefExpr

- `TokenLocation ref`：指向 `token_table` 中存放标识符的位置
- `std::vector<int> array_dim`

##### BinaryOperator

- `enmu BinaryOpKind { kAdd, kSub, kMul, kDiv, kRem, kOr, kAnd, kEQ, kNE, kLT, kLE, kGT, kGE }`
- `BinaryOpKind op`
- `ASTLocation LHS, RHS`：`Expr` 类型

##### UnaryOperator

- `enmu UnaryOpKind { kPlus, kMinus, kNot }`
- `ASTLocation sub_expr`：`Expr` 类型

##### CallExpr

- `TokenLocation func`：指向 `token_table` 中存放标识符的位置
- `std::vector<ASTLocation> params`：`DeclRefExpr` 类型

##### ConstExpr

- `int value`
- `ASTLocation expr`：`Expr` 类型
