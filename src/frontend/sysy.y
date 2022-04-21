%{

#include <iostream>

/* Include the definition of YYLTYPE, import SourceManager, ASTManager and
 * RefList. */
#include "frontend/frontend.h"

/* Location type.  */
using YYLTYPE = ast::SourceRange;
#define YYLTYPE_IS_DECLARED 1

/* Maintain ASTNode's token range. */
#define YYLLOC_DEFAULT(Current, Rhs, N)                             \
    do                                                              \
        if (N) {                                                    \
            (Current).begin_line = YYRHSLOC(Rhs, 1).begin_line;     \
            (Current).begin_column = YYRHSLOC(Rhs, 1).begin_column; \
            (Current).end_line = YYRHSLOC(Rhs, N).end_line;         \
            (Current).end_column = YYRHSLOC(Rhs, N).end_column;     \
        } else {                                                    \
            (Current).begin_line = (Current).end_line               \
                = YYRHSLOC(Rhs, 0).end_line;                        \
            (Current).begin_column = (Current).end_column           \
                = YYRHSLOC(Rhs, 0).end_column;                      \
        }                                                           \
    while (0)

#include "util.h"

void yyerror(const char *format, ...);
void yylerror(YYLTYPE location, const char *format, ...);

%}

%glr-parser
%locations

%union{
    ast::TokenLocation token;
    ast::ASTLocation ast;
    std::vector<ast::ASTLocation> *list;
    bool b;
    int integer;
}

%token <token> CONST "const"
%token <token> VOID "void"
%token <token> INT "int"

%token <token> IF "if"
%token <token> ELSE "else"
%token <token> WHILE "while"
%token <token> BREAK "break"
%token <token> CONTINUE "continue"
%token <token> RETURN "return"

%token <token> IDENT
%token <integer> NUMBER

%token <token> OR "||"
%token <token> AND "&&"
%token <token> EQ "=="
%token <token> NE "!="
%token <token> LE "<="
%token <token> GE ">="

%type <ast> CompUnit

%type <list> Decl
%type <token> BType
%type <list> ConstDefList VarDefList
%type <ast> ConstDef VarDef

%type <list> ArrayDimension
%type <ast> ConstArrayInit ArrayInit
%type <list> ConstInitList InitList
%type <ast> ConstInitItem InitItem

%type <ast> FuncDef FuncDecl
%type <b> FuncType
%type <list> FuncFParamList
%type <ast> FuncFParam

%type <ast> Block
%type <list> BlockItemList
%type <ast> BlockItem Stmt LVal
%type <list> ArrayReference

%type <ast> Exp ConstExp Cond
%type <list> FuncRParamList

/* priority and associativity */

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left "||"
%left "&&"
%left "==" "!="
%left '<' "<=" '>' ">="

%left '+' '-'
%left '*' '/' '%'
%nonassoc CALL PLUS MINUS NOT

%%

CompUnit:                   {
                                $$ = ast_manager.AddNode(new ast::TranslationUnit(ast_manager));
                                ast_manager.SetRoot($$);
                            }
    | CompUnit Decl         {
                                $$ = $1;
                                ast_manager.GetRoot().SetRange(@$);
                                for(auto decl : *$2) ast_manager.GetRoot().AddDecl(decl);
                            }
    | CompUnit FuncDef      {
                                $$ = $1;
                                ast_manager.GetRoot().SetRange(@$);
                                ast_manager.GetRoot().AddDecl($2);
                            }
    ;

/* --------------- Variable Declaration and Definition --------------- */

Decl: "const" BType ConstDefList ';'    {
                                            $$ = $3;
                                            for(auto decl : *$$) ast_manager.GetDecl(decl).SetType(ast::Decl::kInt);
                                        }
    | BType VarDefList ';'              {
                                            $$ = $2;
                                            for(auto decl : *$$) ast_manager.GetDecl(decl).SetType(ast::Decl::kInt);
                                        }
    ;

BType: "int"        { $$ = $1; }
    ;

ConstDefList: ConstDef              { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($1); }
    | ConstDefList ',' ConstDef     { $$ = $1; $$->emplace_back($3); }
    ;
VarDefList: VarDef                  { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($1); }
    | VarDefList ',' VarDef         { $$ = $1; $$->emplace_back($3); }
    ;

ConstDef: IDENT '=' ConstExp                    { $$ = ast_manager.AddNode(new ast::VarDecl(ast_manager, @$, $1, $3, {}, true)); }
    | IDENT ArrayDimension '=' ConstArrayInit   { $$ = ast_manager.AddNode(new ast::VarDecl(ast_manager, @$, $1, $4, *$2, true)); }
    ;
VarDef: IDENT                               { $$ = ast_manager.AddNode(new ast::VarDecl(ast_manager, @$, $1)); }
    | IDENT '=' Exp                         { $$ = ast_manager.AddNode(new ast::VarDecl(ast_manager, @$, $1, $3)); }
    | IDENT ArrayDimension                  { $$ = ast_manager.AddNode(new ast::VarDecl(ast_manager, @$, $1, *$2)); }
    | IDENT ArrayDimension '=' ArrayInit    { $$ = ast_manager.AddNode(new ast::VarDecl(ast_manager, @$, $1, $4, *$2)); }
    ;

ArrayDimension: '[' ConstExp ']'        { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($2); }
    | ArrayDimension '[' ConstExp ']'   { $$ = $1; $$->emplace_back($3); }
    ;

ConstArrayInit: '{' ConstInitList '}'       { $$ = ast_manager.AddNode(new ast::InitListExpr(ast_manager, @$, *$2)); }
    ;
ArrayInit: '{' InitList '}'                 { $$ = ast_manager.AddNode(new ast::InitListExpr(ast_manager, @$, *$2)); }
    ;

ConstInitList:                              { $$ = new std::vector<ast::ASTLocation>(); }
    | ConstInitItem                         { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($1); }
    | ConstInitList ',' ConstInitItem       { $$ = $1; $$->emplace_back($3); }
    ;
InitList:                                   { $$ = new std::vector<ast::ASTLocation>(); }
    | InitItem                              { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($1); }
    | InitList ',' InitItem                 { $$ = $1; $$->emplace_back($3); }
    ;

ConstInitItem: ConstExp         { $$ = $1; }
    | '{' ConstInitList '}'     { $$ = ast_manager.AddNode(new ast::InitListExpr(ast_manager, @$, *$2)); }
    ;
InitItem: Exp                   { $$ = $1; }
    | '{' InitList '}'          { $$ = ast_manager.AddNode(new ast::InitListExpr(ast_manager, @$, *$2)); }
    ;

/* --------------- Function Definition --------------- */

FuncDef: FuncDecl Block     { ast_manager.GetNode($$).Cast<ast::FunctionDecl>().SetDef($2); }
    ;

FuncDecl: FuncType IDENT '(' FuncFParamList ')'     {
                                                        $$ = ast_manager.AddNode(new ast::FunctionDecl(ast_manager, @$, $2, *$4));
                                                        ast_manager.GetDecl($$).SetType($1 ? ast::Decl::kVoid : ast::Decl::kInt);
                                                    }
    ;

FuncType: "void"        { $$ = true; }
    | BType             { $$ = false; }
    ;

FuncFParamList:                         { $$ = new std::vector<ast::ASTLocation>(); }
    | FuncFParam                        { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($1); }
    | FuncFParamList ',' FuncFParam     { $$ = $1; $$->emplace_back($3); }
    ;

FuncFParam: BType IDENT                     {
                                                $$ = ast_manager.AddNode(new ast::ParamVarDecl(ast_manager, @$, $2, false));
                                                ast_manager.GetDecl($$).SetType(ast::Decl::kInt);
                                            }
    | BType IDENT '[' ']'                   {
                                                $$ = ast_manager.AddNode(new ast::ParamVarDecl(ast_manager, @$, $2, true));
                                                ast_manager.GetDecl($$).SetType(ast::Decl::kInt);
                                            }
    | BType IDENT '[' ']' ArrayDimension    {
                                                $$ = ast_manager.AddNode(new ast::ParamVarDecl(ast_manager, @$, $2, true, *$5));
                                                ast_manager.GetDecl($$).SetType(ast::Decl::kInt);
                                            }
    ;

/* --------------- Block and Statement --------------- */

Block: '{' BlockItemList '}'    { $$ = ast_manager.AddNode(new ast::CompoundStmt(ast_manager, @$, *$2)); }
    ;

BlockItemList:                          { $$ = new std::vector<ast::ASTLocation>(); }
    | BlockItemList BlockItem           { $$ = $1; $$->emplace_back($2); }

BlockItem : Decl        { $$ = ast_manager.AddNode(new ast::DeclStmt(ast_manager, @$, *$1)); }
    | Stmt              { $$ = $1; }
    ;

Stmt: ';'                   { $$ = ast_manager.AddNode(new ast::NullStmt(ast_manager, @$)); }
    | Exp ';'               { $$ = $1; ast_manager.GetNode($$).SetRange(@$); }
    | LVal '=' Exp ';'      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kAssign, $1, $3)); }
    | Block                 { $$ = $1; }
    | "if" '(' Cond ')' Stmt %prec LOWER_THAN_ELSE  { $$ = ast_manager.AddNode(new ast::IfStmt(ast_manager, @$, $3, $5)); }
    | "if" '(' Cond ')' Stmt "else" Stmt            { $$ = ast_manager.AddNode(new ast::IfStmt(ast_manager, @$, $3, $5, $7)); }
    | "while" '(' Cond ')' Stmt     { $$ = ast_manager.AddNode(new ast::WhileStmt(ast_manager, @$, $3, $5)); }
    | "break" ';'                   { $$ = ast_manager.AddNode(new ast::BreakStmt(ast_manager, @$)); }
    | "continue" ';'                { $$ = ast_manager.AddNode(new ast::ContinueStmt(ast_manager, @$)); }
    | "return" ';'                  { $$ = ast_manager.AddNode(new ast::ReturnStmt(ast_manager, @$)); }
    | "return" Exp ';'              { $$ = ast_manager.AddNode(new ast::ReturnStmt(ast_manager, @$, $2)); }
    ;

LVal: IDENT                     { $$ = ast_manager.AddNode(new ast::DeclRefExpr(ast_manager, @$, $1)); }
    | IDENT ArrayReference      { $$ = ast_manager.AddNode(new ast::DeclRefExpr(ast_manager, @$, $1, *$2)); }
    ;

ArrayReference: '[' Exp ']'         { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($2); }
    | ArrayReference '[' Exp ']'    { $$ = $1; $$->emplace_back($3); }
    ;

/* --------------- Expression --------------- */

Exp: '(' Exp ')'        { $$ = ast_manager.AddNode(new ast::ParenExpr(ast_manager, @$, $2)); }
    | LVal              { $$ = $1; }
    | NUMBER            { $$ = ast_manager.AddNode(new ast::IntegerLiteral(ast_manager, @$, $1)); }
    | Exp '+' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kAdd, $1, $3)); }
    | Exp '-' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kSub, $1, $3)); }
    | Exp '*' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kMul, $1, $3)); }
    | Exp '/' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kDiv, $1, $3)); }
    | Exp '%' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kRem, $1, $3)); }
    | IDENT '(' FuncRParamList ')' %prec CALL   {
                                                    ast::TokenLocation loc = $1;
                                                    if(src_manager.GetTokenText($1) == "starttime"){
                                                        loc = src_manager.AddToken("_sysy_starttime", @1);
                                                        int lineno = @1.begin_line;
                                                        (*$3).emplace_back(ast_manager.AddNode(new ast::IntegerLiteral(ast_manager, @3, lineno)));
                                                    }
                                                    if(src_manager.GetTokenText($1) == "stoptime"){
                                                        loc = src_manager.AddToken("_sysy_stoptime", @1);
                                                        int lineno = @1.begin_line;
                                                        (*$3).emplace_back(ast_manager.AddNode(new ast::IntegerLiteral(ast_manager, @3, lineno)));
                                                    }
                                                    $$ = ast_manager.AddNode(new ast::CallExpr(ast_manager, @$, loc, *$3));
                                                }
    | '+' Exp %prec PLUS    { $$ = ast_manager.AddNode(new ast::UnaryOperator(ast_manager, @$, ast::UnaryOperator::kPlus, $2)); }
    | '-' Exp %prec MINUS   { $$ = ast_manager.AddNode(new ast::UnaryOperator(ast_manager, @$, ast::UnaryOperator::kMinus, $2)); }
    | '!' Exp %prec NOT     { $$ = ast_manager.AddNode(new ast::UnaryOperator(ast_manager, @$, ast::UnaryOperator::kNot, $2)); }
    | Exp "||" Exp      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kOr, $1, $3)); }
    | Exp "&&" Exp      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kAnd, $1, $3)); }
    | Exp "==" Exp      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kEQ, $1, $3)); }
    | Exp "!=" Exp      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kNE, $1, $3)); }
    | Exp '<' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kLT, $1, $3)); }
    | Exp "<=" Exp      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kLE, $1, $3)); }
    | Exp '>' Exp       { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kGT, $1, $3)); }
    | Exp ">=" Exp      { $$ = ast_manager.AddNode(new ast::BinaryOperator(ast_manager, @$, ast::BinaryOperator::kGE, $1, $3)); }
    ;

FuncRParamList:                 { $$ = new std::vector<ast::ASTLocation>(); }
    | Exp                       { $$ = new std::vector<ast::ASTLocation>(); $$->emplace_back($1); }
    | FuncRParamList ',' Exp    { $$ = $1; $$->emplace_back($3); }
    ;

ConstExp: Exp       { $$ = $1; }
    ;

Cond: Exp           { $$ = $1; }
    ;

%%

void yyerror(const char *format, ...) {
    va_list args;
    va_start(args, format);

    std::cout << util::FormatTerminalBold("[ERROR] ", util::kFGBrightRed);
    (void)vfprintf(stderr, format, args);
    std::cout << std::endl;

    va_end(args);
}

void yylerror(YYLTYPE location, const char *format, ...) {
    va_list args;
    va_start(args, format);

    std::cout << util::FormatTerminalBold("[ERROR] ", util::kFGBrightRed);
    (void)vfprintf(stderr, format, args);
    std::cout << ' ' << util::FormatTerminal(location.Dump(), util::kFGYellow)
              << std::endl;

    va_end(args);
}
