%{

#include <stdarg.h>

#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>

#include "utils.h"

extern int yylex();
struct YYLTYPE;

std::string locationDump(YYLTYPE location);
void reduce(std::string nonterminal, std::initializer_list<std::string> list);
void yyerror(const char *format, ...);
void yylerror(YYLTYPE location, const char *format, ...);

%}

%glr-parser
%locations

%token CONST "const"
%token VOID "void"
%token INT "int"

%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token BREAK "break"
%token CONTINUE "continue"
%token RETURN "return"

%token IDENT
%token NUMBER

%token OR "||"
%token AND "&&"
%token EQ "=="
%token NE "!="
%token LE "<="
%token GE ">="

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

CompUnit:                   { reduce("CompUnit", {"⌀"}); }
    | CompUnit Decl         { reduce("CompUnit", {"CompUnit", "Decl"}); }
    | CompUnit FuncDef      { reduce("CompUnit", {"CompUnit", "FuncDef"}); }
    ;

/* --------------- Variable Declaration and Definition --------------- */

Decl: "const" BType ConstDefList ';'        { reduce("Decl", {"\"const\"", "BType", "ConstDefList", "';'"}); }
    | BType VarDefList ';'                  { reduce("Decl", {"BType", "VarDefList", "';'"}); }
    ;

BType: "int"        { reduce("BType", {"\"int\""}); }
    ;

ConstDefList: ConstDef                  { reduce("ConstDefList", {"ConstDef"}); }
    | ConstDefList ',' ConstDef         { reduce("ConstDefList", {"ConstDefList", "','", "ConstDef"}); }
    ;
VarDefList: VarDef                      { reduce("VarDefList", {"VarDef"}); }
    | VarDefList ',' VarDef             { reduce("VarDefList", {"VarDefList", "','", "VarDef"}); }
    ;

ConstDef: IDENT '=' ConstExp                        { reduce("ConstDef", {"IDENT", "'='", "ConstExp"}); }
    | IDENT ArrayDimension '=' ConstArrayInit       { reduce("ConstDef", {"IDENT", "ArrayDimension", "'='", "ConstArrayInit"}); }
    ;
VarDef: IDENT                                       { reduce("VarDef", {"IDENT"}); }
    | IDENT '=' Exp                                 { reduce("VarDef", {"IDENT", "'='", "Exp"}); }
    | IDENT ArrayDimension                          { reduce("VarDef", {"IDENT", "ArrayDimension"}); }
    | IDENT ArrayDimension '=' ArrayInit            { reduce("VarDef", {"IDENT", "ArrayDimension", "'='", "ArrayInit"}); }
    ;

ArrayDimension: '[' ConstExp ']'            { reduce("ArrayDimension", {"'['", "ConstExp", "']'"}); }
    | ArrayDimension '[' ConstExp ']'       { reduce("ArrayDimension", {"ArrayDimension", "'['", "ConstExp", "']'"}); }
    ;

ConstArrayInit: '{' ConstInitList '}'       { reduce("ConstArrayInit", {"'{'", "ConstInitList", "'}'"}); }
    ;
ArrayInit: '{' InitList '}'                 { reduce("ArrayInit", {"'{'", "InitList", "'}'"}); }
    ;

ConstInitList:                              { reduce("ConstInitList", {"⌀"}); }
    | ConstInitItem                         { reduce("ConstInitList", {"ConstInitItem"}); }
    | ConstInitList ',' ConstInitItem       { reduce("ConstInitList", {"ConstInitList", "','", "ConstInitItem"}); }
    ;
InitList:                                   { reduce("InitList", {"⌀"}); }
    | InitItem                              { reduce("InitList", {"InitItem"}); }
    | InitList ',' InitItem                 { reduce("InitList", {"InitList", "','", "InitItem"}); }
    ;

ConstInitItem: ConstExp         { reduce("ConstInitItem", {"ConstExp"}); }
    | ConstArrayInit            { reduce("ConstInitItem", {"ConstArrayInit"}); }
    ;
InitItem: Exp                   { reduce("InitItem", {"Exp"}); }
    | ArrayInit                 { reduce("InitItem", {"ArrayInit"}); }
    ;

/* --------------- Function Definition --------------- */

FuncDef: FuncType IDENT '(' FuncFParamList ')' Block        { reduce("FuncDef", {"FuncType", "IDENT", "'('", "FuncFParamList", "')'", "Block"}); }
    ;

FuncType: "void"        { reduce("FuncType", {"\"void\""}); }
    | BType             { reduce("FuncType", {"BType"}); }
    ;

FuncFParamList:                             { reduce("FuncFParamList", {"⌀"}); }
    | FuncFParam                            { reduce("FuncFParamList", {"FuncFParam"}); }
    | FuncFParamList ',' FuncFParam         { reduce("FuncFParamList", {"FuncFParamList", "','", "FuncFParam"}); }
    ;

FuncFParam: BType IDENT                         { reduce("FuncFParam", {"BType", "IDENT"}); }
    | BType IDENT '[' ']'                       { reduce("FuncFParam", {"BType", "IDENT", "'['", "']'"}); }
    | BType IDENT '[' ']' ArrayDimension        { reduce("FuncFParam", {"BType", "IDENT", "'['", "']'", "ArrayDimension"}); }
    ;

/* --------------- Block and Statement --------------- */

Block: '{' BlockItemList '}'        { reduce("Block", {"'{'", "BlockItemList", "'}'"}); }
    ;

BlockItemList:                          { reduce("BlockItemList", {"⌀"}); }
    | BlockItemList BlockItem           { reduce("BlockItemList", {"BlockItemList", "BlockItem"}); }

BlockItem : Decl        { reduce("BlockItem", {"Decl"}); }
    | Stmt              { reduce("BlockItem", {"Stmt"}); }
    ;

Stmt: ';'                               { reduce("Stmt", {"';'"}); }
    | Exp ';'                           { reduce("Stmt", {"Exp", "';'"}); }
    | LVal '=' Exp ';'                  { reduce("Stmt", {"LVal", "'='", "Exp", "';'"}); }
    | Block                             { reduce("Stmt", {"Block"}); }
    | "if" '(' Cond ')' Stmt %prec LOWER_THAN_ELSE      { reduce("Stmt", {"\"if\"", "'('", "Cond", "')'", "Stmt"}); }
    | "if" '(' Cond ')' Stmt "else" Stmt                { reduce("Stmt", {"\"if\"", "'('", "Cond", "')'", "Stmt", "\"else\"", "Stmt"}); }
    | "while" '(' Cond ')' Stmt             { reduce("Stmt", {"\"while\"", "'('", "Cond", "')'", "Stmt"}); }
    | "break" ';'                           { reduce("Stmt", {"break", "';'"}); }
    | "continue" ';'                        { reduce("Stmt", {"continue", "';'"}); }
    | "return" ';'                          { reduce("Stmt", {"return", "';'"}); }
    | "return" Exp ';'                      { reduce("Stmt", {"return", "Exp", "';'"}); }
    ;

LVal: IDENT                             { reduce("LVal", {"IDENT"}); }
    | IDENT ArrayReference              { reduce("LVal", {"IDENT", "ArrayReference"}); }
    ;

ArrayReference: '[' Exp ']'             { reduce("ArrayReference", {"'['", "Exp", "']'"}); }
    | ArrayReference '[' Exp ']'        { reduce("ArrayReference", {"ArrayReference", "'['", "Exp", "']'"}); }
    ;

/* --------------- Expression --------------- */

Exp: '(' Exp ')'        { reduce("Exp", {"'('", "Exp", "')'"}); }
    | LVal              { reduce("Exp", {"LVal"}); }
    | NUMBER            { reduce("Exp", {"NUMBER"}); }
    | Exp '+' Exp           { reduce("Exp", {"Exp", "'+'", "Exp"}); }
    | Exp '-' Exp           { reduce("Exp", {"Exp", "'-'", "Exp"}); }
    | Exp '*' Exp           { reduce("Exp", {"Exp", "'*'", "Exp"}); }
    | Exp '/' Exp           { reduce("Exp", {"Exp", "'/'", "Exp"}); }
    | Exp '%' Exp           { reduce("Exp", {"Exp", "'%'", "Exp"}); }
    | IDENT '(' FuncRParamList ')' %prec CALL       { reduce("Exp", {"IDENT", "'('", "FuncRParamList", "')'"}); }
    | '+' Exp %prec PLUS                            { reduce("Exp", {"'+'", "Exp"}); }
    | '-' Exp %prec MINUS                           { reduce("Exp", {"'-'", "Exp"}); }
    | '!' Exp %prec NOT                             { reduce("Exp", {"'!'", "Exp"}); }
    | Exp "||" Exp          { reduce("Exp", {"Exp", "\"||\"", "Exp"}); }
    | Exp "&&" Exp          { reduce("Exp", {"Exp", "\"&&\"", "Exp"}); }
    | Exp "==" Exp          { reduce("Exp", {"Exp", "\"==\"", "Exp"}); }
    | Exp "!=" Exp          { reduce("Exp", {"Exp", "\"!=\"", "Exp"}); }
    | Exp '<' Exp           { reduce("Exp", {"Exp", "'<'", "Exp"}); }
    | Exp "<=" Exp          { reduce("Exp", {"Exp", "\"<=\"", "Exp"}); }
    | Exp '>' Exp           { reduce("Exp", {"Exp", "'>'", "Exp"}); }
    | Exp ">=" Exp          { reduce("Exp", {"Exp", "\">=\"", "Exp"}); }
    ;

FuncRParamList:                     { reduce("FuncRParamList", {"⌀"}); }
    | Exp                           { reduce("FuncRParamList", {"Exp"}); }
    | FuncRParamList ',' Exp        { reduce("FuncRParamList", {"FuncRParamList", "','", "Exp"}); }
    ;

ConstExp: Exp       { reduce("ConstExp", {"Exp"}); }
    ;

Cond: Exp           { reduce("Cond", {"Exp"}); }
    ;

%%

std::string locationDump(YYLTYPE location) {
    std::ostringstream result;
    result << "<line:" << location.first_line << ':' << location.first_column
           << ", ";
    if (location.first_line == location.last_line) {
        result << "col:" << location.last_column << '>';
    } else {
        result << "line:" << location.last_line << ':' << location.last_column
               << '>';
    }
    return result.str();
}

void reduce(std::string nonterminal, std::initializer_list<std::string> list) {
    std::cout << nonterminal
              << shell::Format(" <---", shell::kFGGreen, shell::kBGDefault, {});
    for (const auto &s : list) { std::cout << ' ' << s; }
    std::cout << std::endl;
}

void yyerror(const char *format, ...) {
    va_list args;
    va_start(args, format);

    std::cout << shell::Format("[ERROR] ", shell::kFGBrightRed,
                               shell::kBGDefault, {shell::kBold});
    vfprintf(stderr, format, args);
    std::cout << std::endl;

    va_end(args);
}

void yylerror(YYLTYPE location, const char *format, ...) {
    va_list args;
    va_start(args, format);

    std::cout << shell::Format("[ERROR] ", shell::kFGBrightRed,
                               shell::kBGDefault, {shell::kBold});
    vfprintf(stderr, format, args);
    std::cout << ' '
              << shell::Format(locationDump(location), shell::kFGYellow,
                               shell::kBGDefault, {})
              << std::endl;

    va_end(args);
}
