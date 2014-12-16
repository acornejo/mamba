%{
#include <string>
#include "types.h"
#include "ast.h"

extern ast::Node *program;
extern void yyerror(void *, const char *);

// NOTE: This requires bison 2.6, on bison 2.3 the %defines directive
// will fail
// for better error reporting
// %define parse.lac full 
// for error reporting
// %locations
// %define api.pure full
// yyerror receives extra YYLTYPE

%}

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
%token<token> CMP_LT CMP_LE CMP_GT CMP_GE CMP_EQ CMP_NE
%token<token> OP_ADD OP_SUB OP_MUL OP_DIV OP_IDIV OP_MOD OP_POW
%token<token> BIT_LSHIFT BIT_RSHIFT BIT_AND BIT_OR BIT_XOR BIT_NEG
%token<token> FALSE TRUE NONE OR AND NOT IF ELSE ELIF WHILE BREAK CONTINUE FOR IN IS NOTIN ISNOT LET RETURN RETURNS DEL

%left OR
%left AND
%right NOT
%left CMP_LT CMP_LE CMP_GT CMP_GE CMP_EQ CMP_NE IN IS ISNOT NOTIN
%left BIT_OR
%left BIT_XOR 
%left BIT_AND
%left BIT_LSHIFT BIT_RSHIFT 
%left OP_ADD OP_SUB
%left OP_MUL OP_DIV OP_IDIV OP_MOD
%right BIT_NEG
%right OP_POW

%type<token> cmp_op bitshift_op arith_op term_op
%type<node> suite stmt_block simple_stmt small_stmt_list compound_stmt small_stmt assn_stmt del_stmt break_stmt continue_stmt return_stmt func_stmt while_stmt for_stmt if_stmt elif_stmt wexpr_list_ne wexpr_list_wf expr_list expr_list_ne expr_list_wf dict_expr list_expr call_expr subs_expr wexpr expr ident_list ident_list_wf pair_expr pair_list pair_list_wf sexpr not_expr and_expr comp_expr bitor_expr bitand_expr bitxor_expr bitshift_expr arith_expr term_expr power_expr

%start program

%{
#include "lexer.h"
%}


%%

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
compound_stmt: func_stmt                                 { $$ = new ast::StmtList(); $$->appendChild($1); }
             | if_stmt                                   { $$ = new ast::StmtList(); $$->appendChild($1); }
             | while_stmt                                { $$ = new ast::StmtList(); $$->appendChild($1); }
             | for_stmt                                  { $$ = new ast::StmtList(); $$->appendChild($1); }
             ;
small_stmt: expr                                         { $$ = new ast::ExprStatement($1); }
          | assn_stmt                                    { $$ = $1; }
          | del_stmt                                     { $$ = $1; }
          | break_stmt                                   { $$ = $1; }
          | continue_stmt                                { $$ = $1; }
          | return_stmt                                  { $$ = $1; }
          ;
break_stmt: BREAK                                        { $$ = new ast::Break(); }
          ;
continue_stmt: CONTINUE                                  { $$ = new ast::Continue(); }
             ;
return_stmt: RETURN                                      { $$ = new ast::Return(new ast::None()); }
           | RETURN expr                                 { $$ = new ast::Return($2); }
           /* FIX: Add support for returning expr_list, should wrap in list */
           ;
assn_stmt: wexpr '=' expr                                { $$ = new ast::Assign($1, $3); }
         | wexpr '=' assn_stmt                           { $$ = new ast::Assign($1, $3); }
         /* FIX: Add support for assigning lists to lists, and for
          * unpacking (unpacking requires the UNPACK_SEQUENCE [2]
          * opcode. */
         ;
del_stmt: DEL wexpr_list_ne                              { $$ = new ast::Delete($2); }
        ;
ident_list: /* empty */                                  { $$ = new ast::IdentList(); }
          | ident_list_wf                                { $$ = $1; }
          ;
ident_list_wf: IDENTIFIER                                { $$ = new ast::IdentList(); $$->appendChild(new ast::Variable($1)); }
         | ident_list_wf ',' IDENTIFIER                  { $$ = $1; $1->appendChild(new ast::Variable($3)); }
         ; 
/* func_stmt: DEF IDENTIFIER '(' ident_list ')' ':' suite   { $$ = new ast::Assign(new ast::Variable($2), new ast::FunctionDef($2, $4, $7)); } */
         ;
if_stmt: IF expr ':' suite elif_stmt                     { $$ = new ast::IfElse($2, $4, $5); }
       ;
elif_stmt: /* empty */                                   { $$ = NULL; }
     | ELIF expr ':' suite elif_stmt                     { $$ = new ast::IfElse($2, $4, $5); }
     | ELSE ':' suite                                    { $$ = $3; }
     ;
while_stmt: WHILE expr ':' suite                         { $$ = new ast::While($2, $4); }
          ;
for_stmt: FOR IDENTIFIER IN expr ':' suite          { $$ = new ast::For(new ast::Variable($2), $4, $6); }
        /*| FOR IDENTIFIER IN expr_list_ne ':' suite       { $$ = new ast::For(new ast::Variable($2), new ast::List($4), $6); }*/
        ;
wexpr_list_ne: wexpr_list_wf                             { $$ = $1; }
             | wexpr_list_wf ','                         { $$ = $1; }
             ;
wexpr_list_wf: wexpr                                     { $$ = new ast::ExprList(); $$->appendChild($1); } 
             | wexpr_list_wf ',' wexpr                   { $$ = $1; $$->appendChild($3); }
             ;
wexpr: IDENTIFIER                                        { $$ = new ast::Variable($1); }
     | IDENTIFIER '[' expr ']'                           { $$ = new ast::Subscript(new ast::Variable($1), $3); }
     ;
expr_list: /* empty */                                   { $$ = new ast::ExprList(); }
         | expr_list_ne                                  { $$ = $1; }
         ;
expr_list_ne: expr_list_wf                               { $$ = $1; }
            | expr_list_wf ','                           { $$ = $1; }
            ;
expr_list_wf: expr                                       { $$ = new ast::ExprList(); $$->appendChild($1); }
            | expr_list_wf ',' expr                      { $$ = $1; $1->appendChild($3); }
            ;
pair_list: /* empty */                                   { $$ = new ast::PairList(); }
         | pair_list_wf                                  { $$ = $1; }
         | pair_list_wf ','                              { $$ = $1; }
         ;
pair_list_wf: pair_expr                                  { $$ = new ast::PairList(); $$->appendChild($1); }
            | pair_list_wf ',' pair_expr                 { $$ = $1; $1->appendChild($3); }
            ;
pair_expr: expr ':' expr                                 { $$ = new ast::Pair($1, $3); }
         ;
dict_expr: '{' pair_list '}'                             { $$ = new ast::Dict($2); }
         ;
list_expr: '[' expr_list ']'                             { $$ = new ast::List($2); }
         ;
call_expr: IDENTIFIER '(' expr_list ')'                  { $$ = new ast::Call(new ast::Variable($1), $3); }
         | call_expr '(' expr_list ')'                   { $$ = new ast::Call($1, $3); }
         ;
subs_expr: IDENTIFIER '[' expr ']'                       { $$ = new ast::Subscript(new ast::Variable($1), $3); }
         | dict_expr '[' expr ']'                        { $$ = new ast::Subscript($1, $3); }
         | list_expr '[' expr ']'                        { $$ = new ast::Subscript($1, $3); }
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
cmp_op: CMP_LT                                           { $$ = $1; }
      | CMP_LE                                           { $$ = $1; }
      | CMP_GT                                           { $$ = $1; }
      | CMP_GE                                           { $$ = $1; }
      | CMP_EQ                                           { $$ = $1; }
      | CMP_NE                                           { $$ = $1; }
      | IS                                               { $$ = $1; }
      | ISNOT                                            { $$ = $1; }
      | IN                                               { $$ = $1; }
      | NOTIN                                            { $$ = $1; }
      ;
comp_expr: bitor_expr                                    { $$ = $1; }
         | comp_expr cmp_op bitor_expr                   { $$ = new ast::Binary($2, $1, $3); }
         ;
bitor_expr: bitxor_expr                                  { $$ = $1; }
          | bitor_expr BIT_OR bitxor_expr                { $$ = new ast::Binary($2, $1, $3); }
          ;
bitxor_expr: bitand_expr                                 { $$ = $1; }
           | bitxor_expr BIT_XOR bitand_expr             { $$ = new ast::Binary($2, $1, $3); }
           ;
bitand_expr: bitshift_expr                               { $$ = $1; }
           | bitand_expr BIT_AND bitshift_expr           { $$ = new ast::Binary($2, $1, $3); }
           ;
bitshift_op: BIT_LSHIFT                                  { $$ = $1; }
           | BIT_RSHIFT                                  { $$ = $1; }
           ;
bitshift_expr: arith_expr                                { $$ = $1; }
             | bitshift_expr bitshift_op arith_expr      { $$ = new ast::Binary($2, $1, $3); }
             ;
arith_op: OP_ADD                                         { $$ = $1; }
        | OP_SUB                                         { $$ = $1; }
        ;
term_op: OP_MUL                                          { $$ = $1; }
       | OP_DIV                                          { $$ = $1; }
       | OP_MOD                                          { $$ = $1; }
       | OP_IDIV                                         { $$ = $1; }
       ;
arith_expr: term_expr                                    { $$ = $1; }
          | arith_expr arith_op term_expr                { $$ = new ast::Binary($2, $1, $3); }
          ;
term_expr: power_expr                                    { $$ = $1; }
         | term_expr term_op power_expr                  { $$ = new ast::Binary($2, $1, $3); }
         ;
power_expr: sexpr                                        { $$ = $1; }
          | power_expr OP_POW sexpr                      { $$ = new ast::Binary($2, $1, $3); }
          ;
sexpr: OP_ADD sexpr %prec BIT_NEG                        { $$ = new ast::Unary($1, $2); }
     | OP_SUB sexpr %prec BIT_NEG                        { $$ = new ast::Unary($1, $2); }
     | BIT_NEG sexpr                                     { $$ = new ast::Unary($1, $2); }
     | '(' expr ')'                                      { $$ = $2; }
     | list_expr                                         { $$ = $1; }
     | dict_expr                                         { $$ = $1; }
     | call_expr                                         { $$ = $1; }
     | subs_expr                                         { $$ = $1; }
     | TRUE                                              { $$ = new ast::True(); }
     | FALSE                                             { $$ = new ast::False(); }
     | NONE                                              { $$ = new ast::None(); }
     | INTEGER                                           { $$ = new ast::Integer($1); }
     | REAL                                              { $$ = new ast::Real($1); }
     | STRING                                            { $$ = new ast::String($1); }
     | IDENTIFIER                                        { $$ = new ast::Variable($1); }
     ;
%%
