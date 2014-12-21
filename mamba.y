%{
#include <string>
#include "types.h"
#include "ast.h"

extern ast::Node *program;
extern void yyerror(void *, const char *);

// for better error reporting
// %define parse.lac full
// for error reporting
// %locations
// %define api.pure full
// yyerror receives extra YYLTYPE

%}

%require "2.5"
%defines "parser.h"
%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%union {
    double real;
    int64_t integer;
    int token;
    std::string *string;
    ast::Node *node;
}

%token<string> IDENTIFIER STRING
%token<real> REAL
%token<integer> INTEGER
%token<token> INDENT DEDENT NEWLINE
%token<token> T_LT T_LE T_GT T_GE T_EQ T_NE
%token<token> T_ADD T_SUB T_MUL T_DIV T_MOD T_POW
%token<token> T_LSHIFT T_RSHIFT T_BITAND T_BITOR T_BITXOR T_BITNEG T_RETURNS
%token<token> LET FALSE TRUE TUPLE RECORD UNION OR AND NOT IF ELSE ELIF WHILE BREAK CONTINUE FOR IN RETURN

%left OR
%left AND
%right NOT
%left T_LE T_GE T_EQ T_NE
%left T_BITXOR
%left T_BITAND
%left T_LSHIFT T_RSHIFT
%left T_SUB
%left T_MUL T_DIV T_MOD
%right T_BITNEG
%right T_POW

%type<token> cmp_op bitshift_op arith_op term_op
%type<node> suite stmt_block simple_stmt small_stmt_list compound_stmt small_stmt assn_stmt decl_stmt break_stmt continue_stmt return_stmt while_stmt for_stmt if_stmt elif_stmt func_expr fexpr_list expr_list array_expr call_expr subs_expr fexpr wexpr expr sexpr not_expr and_expr comp_expr bitor_expr bitand_expr bitxor_expr bitshift_expr arith_expr term_expr power_expr record_block record_suite type_stmt record_def tuple_block tuple_suite tuple_def union_decl union_block union_suite union_def type_spec type_decl type_decl_list 

%start program

%{
#include "lexer.h"
%}


%%
cmp_op: '<'                                              { $$ = T_LT; }
      | '>'                                              { $$ = T_GT; }
      | T_LE                                             { $$ = $1; }
      | T_GE                                             { $$ = $1; }
      | T_EQ                                             { $$ = $1; }
      | T_NE                                             { $$ = $1; }
      ;

bitshift_op: T_LSHIFT                                    { $$ = $1; }
           | T_RSHIFT                                    { $$ = $1; }
           ;

arith_op: '+'                                            { $$ = T_ADD; }
        | '-'                                            { $$ = T_SUB; }
        ;

term_op: '*'                                             { $$ = T_MUL; }
       | '/'                                             { $$ = T_DIV; }
       | '%'                                             { $$ = T_MOD; }
       ;

program: stmt_block                                      { program = $1;}
       ;

suite: NEWLINE INDENT stmt_block DEDENT                  { $$ = $3; }
     | simple_stmt                                       { $$ = $1; }
     ;

stmt_block: simple_stmt                                  { $$ = $1; }
          | compound_stmt                                { $$ = $1; }
          | stmt_block simple_stmt                       { $$ = $1; $$->extend($2); }
          | stmt_block compound_stmt                     { $$ = $1; $$->extend($2); }
          ;

simple_stmt: small_stmt_list NEWLINE                     { $$ = $1; }
           | small_stmt_list ';' NEWLINE                 { $$ = $1; }
           ;

small_stmt_list: small_stmt                              { $$ = new ast::StmtList(); $$->appendChild($1); }
               | small_stmt_list ';' small_stmt          { $$ = $1; $$->appendChild($3); }
               ;

compound_stmt: if_stmt                                   { $$ = new ast::StmtList(); $$->appendChild($1); }
             | while_stmt                                { $$ = new ast::StmtList(); $$->appendChild($1); }
             | for_stmt                                  { $$ = new ast::StmtList(); $$->appendChild($1); }
             ;

small_stmt: expr                                         { $$ = new ast::ExprStatement($1); }
          | decl_stmt                                    { $$ = $1; }
          | type_stmt                                    { $$ = $1; }
          | assn_stmt                                    { $$ = $1; }
          | break_stmt                                   { $$ = $1; }
          | continue_stmt                                { $$ = $1; }
          | return_stmt                                  { $$ = $1; }
          ;

break_stmt: BREAK                                        { $$ = new ast::Break(); }
          ;

continue_stmt: CONTINUE                                  { $$ = new ast::Continue(); }
             ;

return_stmt: RETURN                                      { $$ = new ast::Return(NULL); }
           | RETURN expr                                 { $$ = new ast::Return($2); }
           ;

assn_stmt: wexpr '=' expr                                { $$ = new ast::Assign($1, $3); }
         | wexpr '=' assn_stmt                           { $$ = new ast::Assign($1, $3); }
         ;

type_spec: IDENTIFIER                                    { $$ = new ast::TypeSpec($1); }
         | '*' type_spec                                 { ((ast::TypeSpec*)$2)->setPointer(); }
         | '&' type_spec                                 { ((ast::TypeSpec*)$2)->setReference(); }
         | type_spec '[' ']'                             { ((ast::TypeSpec*)$1)->pushArray(-1); }
         | type_spec '[' INTEGER ']'                     { ((ast::TypeSpec*)$1)->pushArray($3); }
         ;

decl_stmt: LET IDENTIFIER '=' fexpr                      { $$ = new ast::Declare($2, $4, NULL); }
         | LET type_spec IDENTIFIER '=' fexpr            { $$ = new ast::Declare($3, $5, $2); }
         ;

if_stmt: IF expr ':' suite elif_stmt                     { $$ = new ast::IfElse($2, $4, $5); }
       ;

elif_stmt: /* empty */                                   { $$ = NULL; }
     | ELIF expr ':' suite elif_stmt                     { $$ = new ast::IfElse($2, $4, $5); }
     | ELSE ':' suite                                    { $$ = $3; }
     ;

while_stmt: WHILE expr ':' suite                         { $$ = new ast::While($2, $4); }
          ;

for_stmt: FOR IDENTIFIER IN expr ':' suite               { $$ = new ast::For(new ast::Variable($2), $4, $6); }
        ;

wexpr: IDENTIFIER                                        { $$ = new ast::Variable($1); }
     | IDENTIFIER '[' expr ']'                           { $$ = new ast::Subscript(new ast::Variable($1), $3); }
     ;

type_decl: type_spec IDENTIFIER                          { $$ = new ast::TypeDecl($1, $2); }
         ;

type_decl_list: /* empty */                              { $$ = new ast::TypeDeclList(); }
              | type_decl                                { $$ = new ast::TypeDeclList(); $$->appendChild($1); }
              | type_decl_list ',' type_decl             { $$ = $1; $1->appendChild($3); }
              ;

func_expr: '|' type_decl_list '|' T_RETURNS type_spec ':' suite
         { $$ = new ast::FunctionExpr($2, $5, $7); }
         | '|' type_decl_list '|' ':' suite
         { $$ = new ast::FunctionExpr($2, NULL, $5); }
         ;

record_block: type_decl NEWLINE                          { $$ = new ast::TypeDeclList(); $$->appendChild($1); }
            | record_block type_decl NEWLINE             { $$ = $1; $1->appendChild($3); }
            ;

record_suite: NEWLINE INDENT record_block DEDENT         { $$ = $3; }
            ;

record_def: RECORD IDENTIFIER ':' record_suite           { $$ = new ast::RecordDef($2, $4); }
          ;

tuple_block: type_spec NEWLINE                           { $$ = new ast::TupleTypes(); $$->appendChild($1); }
           | tuple_block type_spec                       { $$ = $1; $1->appendChild($2); }
           ;

tuple_suite: NEWLINE INDENT tuple_block DEDENT           { $$ = $3; }
           ;

tuple_def: TUPLE IDENTIFIER ':' tuple_suite              { $$ = new ast::TupleDef($2, $4); }
         ;

union_decl: IDENTIFIER                                   { $$ = new ast::UnionItem($1, NULL); }
          | IDENTIFIER '(' type_spec ')'                 { $$ = new ast::UnionItem($1, $3); }
          ;

union_block: union_decl NEWLINE                          { $$ = new ast::UnionList(); $$->appendChild($1); }
           | union_block union_decl                      { $$ = $1; $1->appendChild($2); }
           ;

union_suite: NEWLINE INDENT union_block DEDENT           { $$ = $3; }
           ;

union_def: UNION IDENTIFIER ':' union_suite              { $$ = new ast::UnionDef($2, $4); }
         ;

type_stmt: record_def                                    { $$ = $1; }
         | tuple_def                                     { $$ = $1; }
         | union_def                                     { $$ = $1; }
         ;

expr_list: expr                                          { $$ = new ast::ExprList(); $$->appendChild($1); }
         | expr_list ',' expr                            { $$ = $1; $1->appendChild($3); }
         ;

array_expr: '[' expr_list ']'                            { $$ = new ast::Array($2); }
         ;

fexpr: expr                                              { $$ = $1; }
     | func_expr                                         { $$ = $1; }
     ;

fexpr_list: fexpr                                        { $$ = new ast::ExprList(); $$->appendChild($1); }
          | fexpr_list ',' fexpr                         { $$ = $1; $1->appendChild($3); }
          ;

call_expr: IDENTIFIER '(' fexpr_list ')'                  { $$ = new ast::Call(new ast::Variable($1), $3); }
         | call_expr '(' fexpr_list ')'                   { $$ = new ast::Call($1, $3); }
         ;

subs_expr: IDENTIFIER '[' expr ']'                       { $$ = new ast::Subscript(new ast::Variable($1), $3); }
         | array_expr '[' expr ']'                       { $$ = new ast::Subscript($1, $3); }
         | call_expr '[' expr ']'                        { $$ = new ast::Subscript($1, $3); }
         | subs_expr '[' expr ']'                        { $$ = new ast::Subscript($1, $3); }
         ;

expr: and_expr                                           { $$ = $1; }
    | expr OR and_expr                                   { $$ = new ast::Or($1, $3); }
    ;

and_expr: not_expr                                       { $$ = $1; }
       | and_expr AND not_expr                           { $$ = new ast::And($1, $3); }
       ;

not_expr: comp_expr                                      { $$ = $1; }
        | NOT comp_expr                                  { $$ = new ast::Unary($1, $2); }
        ;

comp_expr: bitor_expr                                    { $$ = $1; }
         | comp_expr cmp_op bitor_expr                   { $$ = new ast::Binary($2, $1, $3); }
         ;

bitor_expr: bitxor_expr                                  { $$ = $1; }
          | bitor_expr '|' bitxor_expr                   { $$ = new ast::Binary(T_BITOR, $1, $3); }
          ;

bitxor_expr: bitand_expr                                 { $$ = $1; }
           | bitxor_expr '^' bitand_expr                 { $$ = new ast::Binary(T_BITXOR, $1, $3); }
           ;

bitand_expr: bitshift_expr                               { $$ = $1; }
           | bitand_expr '&' bitshift_expr               { $$ = new ast::Binary(T_BITAND, $1, $3); }
           ;

bitshift_expr: arith_expr                                { $$ = $1; }
             | bitshift_expr bitshift_op arith_expr      { $$ = new ast::Binary($2, $1, $3); }
             ;

arith_expr: term_expr                                    { $$ = $1; }
          | arith_expr arith_op term_expr                { $$ = new ast::Binary($2, $1, $3); }
          ;

term_expr: power_expr                                    { $$ = $1; }
         | term_expr term_op power_expr                  { $$ = new ast::Binary($2, $1, $3); }
         ;

power_expr: sexpr                                        { $$ = $1; }
          | power_expr T_POW sexpr                       { $$ = new ast::Binary($2, $1, $3); }
          ;

sexpr: '+' sexpr %prec T_BITNEG                          { $$ = new ast::Unary(T_ADD, $2); }
     | '-' sexpr %prec T_BITNEG                          { $$ = new ast::Unary(T_SUB, $2); }
     | T_BITNEG sexpr                                    { $$ = new ast::Unary($1, $2); }
     | '(' expr ')'                                      { $$ = $2; }
     | array_expr                                        { $$ = $1; }
     | call_expr                                         { $$ = $1; }
     | subs_expr                                         { $$ = $1; }
     | TRUE                                              { $$ = new ast::True(); }
     | FALSE                                             { $$ = new ast::False(); }
     | INTEGER                                           { $$ = new ast::Integer($1); }
     | REAL                                              { $$ = new ast::Real($1); }
     | STRING                                            { $$ = new ast::String($1); }
     | IDENTIFIER                                        { $$ = new ast::Variable($1); }
     ;
%%
