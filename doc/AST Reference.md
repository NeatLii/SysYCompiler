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

// index in token table
using TokenLocation = std::vector<Token>::size_type;

class SourceMageger {
    std::string file_name;
	std::vector<Token> token_table;
}
```



# AST Manager

保存所有 AST 结点。

```c++
// index in ASTNode table
using ASTLocation = std::vector<ASTNode>::size_type;

class ASTManager {
	SourceManager &raw;
    std::vector<std::unique_ptr<ASTNode>> node_table;
    bool has_root;
    ASTLocation root;
}
```



# Ident Table

```c++
class IdentTable {
    std::unordered_map<std::string, ASTLocation> ident_table;
}
```



# ASTNode (virtual)

```c++
class ASTNode : public IdentTable {
    enum ASTNodeKind {};
    ASTNodeKind kind;
    
    ASTManager &src;
    SourceRange range;

    bool has_location;
    ASTLocation location;
    bool has_parent;
    ASTLocation parent;
}
```



## TranslationUnit

```c++
class TranslationUnit final : public ASTNode {
    std::vector<ASTLocation> decl_list;		// Decl
}
```



## Decl (virtual)

```c++
class Decl : public ASTNode {
    enum Type { kUndef, kVoid, kInt };
    Type type;
    TokenLocation ident;
}
```

### VarDecl

```c++
class VarDecl final : public Decl {
    bool has_init;
    ASTLocation init;	// Expr
    
    std::vector<ASTLocation> arr_dim_list;	// Expr
    bool is_const;
}
```

### ParamVarDecl

```c++
class ParamVarDecl final : public Decl {
    bool is_ptr;
    std::vector<ASTLocation> arr_dim_list;	// Expr
}
```

### FunctionDecl

```c++
class FunctionDecl final : public Decl {
    std::vector<ASTLocation> param_list;	// ParamVarDecl
    bool has_def;
    ASTLocation def;						// CompoundStmt
}
```



## Stmt (virtual)

```c++
class Stmt : public ASTNode {}
```

### CompoundStmt

```c++
class CompoundStmt final : public Stmt {
    std::vector<ASTLocation> stmt_list;		// Stmt
}
```

### DeclStmt

```c++
class DeclStmt final : public Stmt {
    std::vector<ASTLocation> decl_list;		// VarDecl
}
```

### NullStmt

```c++
class NullStmt final : public Stmt {}
```

### IfStmt

```c++
class IfStmt final : public Stmt {
    ASTLocation cond;			// Expr
    ASTLocation if_true;		// Stmt
    bool has_else;
    ASTLocation if_false;		// Stmt
}
```

### WhileStmt

```c++
class WhileStmt final : public Stmt {
    ASTLocation cond;		// Expr
    ASTLocation body;		// Stmt
}
```

### ContinueStmt

```c++
class ContinueStmt final : public Stmt {}
```

### BreakStmt

```c++
class BreakStmt final : public Stmt {}
```

### ReturnStmt

```c++
class ReturnStmt final : public Stmt {
    bool has_expr;
    ASTLocation expr;		// Expr
}
```

### Expr (virtual)

```c++
class Expr : public Stmt {
    bool is_const;
    int value;
}
```

##### IntegerLiteral

```c++
class IntegerLiteral final : public Expr {
    bool is_filler;
}
```

##### ParenExpr

```c++
class ParenExpr final : public Expr {
    ASTLocation sub_expr;	// Expr
}
```

##### DeclRefExpr

```c++
class DeclRefExpr final : public Expr {
    TokenLocation ident;
    std::vector<ASTLocation> arr_dim_list;	// Expr

    bool has_ref;
    ASTLocation ref;						// Decl
}
```

##### CallExpr

```c++
class CallExpr final : public Expr {
    TokenLocation ident;
    std::vector<ASTLocation> param_list;	// Expr

    bool has_ref;
    ASTLocation ref;						// FunctionDecl
}
```

##### BinaryOperator

```c++
class BinaryOperator final : public Expr {
    enum BinaryOpKind {};
    BinaryOpKind op_code;
    ASTLocation LHS;		// Expr
    ASTLocation RHS;		// Expr
}
```

##### UnaryOperator

```c++
class UnaryOperator final : public Expr {
    enum UnaryOpKind {};
    UnaryOpKind op_code;
    ASTLocation sub_expr;	// Expr
}
```

##### InitListExpr

```c++
class InitListExpr final : public Expr {
    std::vector<ASTLocation> init_list;		// Expr
    
    std::vector<int> format;
     bool is_filler;
}
```

