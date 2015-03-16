%{
#include <string>
#include "mamba_context.h"
#include "lexer.h"

// hack to send scanner to yylex
#define context_scanner context->getScanner()
%}

%require "2.5"
%defines "parser.h"
%output  "parser.cc"
%define api.pure full
%define parse.error verbose
%locations
%parse-param { MambaContext *context }
%lex-param { MambaContext *context_scanner }
%define parse.trace

%union {
    double real;
    long int integer;
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
%token<token> T_LSHIFT T_RSHIFT T_BITAND T_BITOR T_BITXOR T_BITNEG T_ARROW T_ELLIPSIS
%token<token> VAR FUN FALSE TRUE RECORD UNION OR AND NOT IF ELSE ELIF WHILE BREAK CONTINUE FOR IN RETURN

/* Clean up memory in case of error */
%destructor { delete $$; } <node>
%destructor { delete $$; } IDENTIFIER
%destructor { delete $$; } STRING

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
%type<node> suite stmt_block simple_stmt small_stmt compound_stmt assn_stmt decl_stmt func_stmt break_stmt continue_stmt return_stmt while_stmt for_stmt if_stmt elif_stmt func_expr expr_list_ne expr_list array_expr call_expr subs_expr wexpr expr sexpr not_expr and_expr comp_expr bitor_expr bitand_expr bitxor_expr bitshift_expr arith_expr term_expr power_expr record_block record_suite record_stmt union_decl union_block union_suite union_stmt pointer_type array_type ref_type tuple_type func_type return_type type type_list type_list_ne type_decl type_decl_list

%start program

%%
program:
    stmt_block
    { context->setOutput($1); };

suite:
    NEWLINE INDENT stmt_block DEDENT
    { $$ = $3; } ;

stmt_block:
    compound_stmt
    { $$ = new ast::StmtList(); $$->appendChild($1); } |

    simple_stmt
    { $$ = new ast::StmtList(); $$->appendChild($1); } |

    stmt_block simple_stmt
    { $$ = $1; $$->appendChild($2); } |

    stmt_block compound_stmt
    { $$ = $1; $$->appendChild($2); } ;

simple_stmt:
    small_stmt NEWLINE
    { $$ = $1; } ;

small_stmt:
    expr
    { $$ = new ast::Expr($1); } |

    decl_stmt
    { $$ = $1; } |

    assn_stmt
    { $$ = $1; } |

    break_stmt
    { $$ = $1; } |

    continue_stmt
    { $$ = $1; } |

    return_stmt
    { $$ = $1; } ;

compound_stmt:
    if_stmt
    { $$ = $1; } |

    while_stmt
    { $$ = $1; } |

    for_stmt
    { $$ = $1; } |

    func_stmt
    { $$ = $1; } |

    record_stmt
    { $$ = $1; } |

    union_stmt
    { $$ = $1; } ;

break_stmt:
    BREAK
    { $$ = new ast::Break(); } ;

continue_stmt:
    CONTINUE
    { $$ = new ast::Continue(); } ;

return_stmt:
    RETURN
    { $$ = new ast::Return(NULL); } |

    RETURN expr
    { $$ = new ast::Return($2); } ;

assn_stmt:
    wexpr '=' expr
    { $$ = new ast::Assign($1, $3); } |

    wexpr '=' assn_stmt
    { $$ = $3; ((ast::Assign*)$$)->vars.push_back($1); } ;

decl_stmt:
    VAR IDENTIFIER '=' expr
    { $$ = new ast::Declaration($2, $4, NULL); } |

    VAR type IDENTIFIER '=' expr
    { $$ = new ast::Declaration($3, $5, $2); } ;

func_stmt:
    FUN IDENTIFIER func_expr
    { $$ = new ast::FuncDecl($2, $3); } ;

if_stmt:
    IF expr ':' suite elif_stmt
    { $$ = new ast::IfElse($2, $4, $5); } ;

elif_stmt:
    %empty
    { $$ = NULL; } |

    ELIF expr ':' suite elif_stmt
    { $$ = new ast::IfElse($2, $4, $5); } |

    ELSE ':' suite
    { $$ = $3; } ;

while_stmt:
    WHILE expr ':' suite
    { $$ = new ast::While($2, $4); } ;

for_stmt:
    FOR IDENTIFIER IN expr ':' suite
    { $$ = new ast::For($2, $4, $6); } ;

record_stmt:
    RECORD IDENTIFIER ':' record_suite
    { $$ = new ast::RecordDef($2, $4); } ;

union_stmt:
    UNION IDENTIFIER ':' union_suite
    { $$ = new ast::UnionDef($2, $4); } ;

record_block:
    type_decl NEWLINE
    { $$ = new ast::TypeDeclList(); $$->appendChild($1); } |

    record_block type_decl NEWLINE
    { $$ = $1; $1->appendChild($2); } ;

record_suite:
    NEWLINE INDENT record_block DEDENT
    { $$ = $3; } ;

union_decl:
    IDENTIFIER
    { $$ = new ast::UnionItem($1, NULL); } |

    IDENTIFIER '(' type ')'
    { $$ = new ast::UnionItem($1, $3); } ;

union_block:
    union_decl NEWLINE
    { $$ = new ast::UnionList(); $$->appendChild($1); } |

    union_block union_decl NEWLINE
    { $$ = $1; $1->appendChild($2); } ;

union_suite:
    NEWLINE INDENT union_block DEDENT
    { $$ = $3; } ;

pointer_type:
    '*' type
    { $$ = new ast::PointerType($2); } ;

array_type:
    '[' ']' type
    { $$ = new ast::ArrayType($3, -1); } |

    '[' INTEGER ']' type
    { $$ = new ast::ArrayType($4, $2); } ;

ref_type:
    '&' type
    { $$ = new ast::ReferenceType($2); } ;

type_list:
    type
    { $$ = new ast::TypeList(); $$->appendChild($1); } |

    type_list ',' type
    { $$ = $1; $$->appendChild($3); } ;

type_list_ne:
    type ',' type
    { $$ = new ast::TypeList(); $$->appendChild($1); $$->appendChild($3); } |

    type_list_ne ',' type
    { $$ = $1; $$->appendChild($3); } ;


tuple_type:
    '(' type_list_ne ')'
    { $$ = new ast::TupleType($2); } ;

return_type:
    T_ARROW '(' ')'
    { $$ = NULL; } |

    T_ARROW type
    { $$ = $2; } ;

func_type:
    '|' '|' return_type
    { $$ = new ast::FuncType(new ast::TypeList(), $3); } |

    '|' type_list '|' return_type
    { $$ = new ast::FuncType($2, $4); } ;

type:
    IDENTIFIER
    { $$ = new ast::SimpleType($1); } |

    pointer_type
    { $$ = $1; } |

    array_type
    { $$ = $1; } |

    tuple_type
    { $$ = $1; } |

    ref_type
    { $$ = $1; } |

    func_type
    { $$ = $1; } ;

type_decl:
    type IDENTIFIER
    { $$ = new ast::TypeDecl($1, $2); } ;

type_decl_list:
    type_decl
    { $$ = new ast::TypeDeclList(); $$->appendChild($1); } |

    type_decl_list ',' type_decl
    { $$ = $1; $1->appendChild($3); } ;

func_expr:
    '|' '|' return_type ':' suite
    { $$ = new ast::Function(new ast::TypeList(), $5, $3); } |

    '|' type_decl_list '|' return_type ':' suite
    { $$ = new ast::Function($2, $6, $4); } ;

wexpr:
    IDENTIFIER
    { $$ = new ast::Variable($1); } |

    IDENTIFIER '[' expr ']'
    { $$ = new ast::Subscript(new ast::Variable($1), $3); } ;

expr_list_ne:
    expr
    { $$ = new ast::ExprList(); $$->appendChild($1); } |

    expr_list_ne ',' expr
    { $$ = $1; $1->appendChild($3); } ;


expr_list:
    %empty
    { $$ = new ast::ExprList(); } |

    expr_list_ne
    { $$ = $1; } ;

array_expr:
    '[' expr_list_ne ']'
    { $$ = new ast::Array($2); } ;

call_expr:
    IDENTIFIER '(' expr_list ')'
    { $$ = new ast::Call(new ast::Variable($1), $3); } |

    call_expr '(' expr_list ')'
    { $$ = new ast::Call($1, $3); } ;

subs_expr:
    IDENTIFIER '[' expr ']'
    { $$ = new ast::Subscript(new ast::Variable($1), $3); } |

    array_expr '[' expr ']'
    { $$ = new ast::Subscript($1, $3); } |

    call_expr '[' expr ']'
    { $$ = new ast::Subscript($1, $3); } |

    subs_expr '[' expr ']'
    { $$ = new ast::Subscript($1, $3); } ;

expr:
    and_expr
    { $$ = $1; } |

    expr OR and_expr
    { $$ = new ast::Or($1, $3); } ;

and_expr:
    not_expr
    { $$ = $1; } |

    and_expr AND not_expr
    { $$ = new ast::And($1, $3); } ;

not_expr:
    comp_expr
    { $$ = $1; } |

    NOT comp_expr
    { $$ = new ast::Unary($1, $2); } ;

cmp_op:
    '<'
    { $$ = T_LT; } |

    '>'
    { $$ = T_GT; } |

    T_LE
    { $$ = $1; } |

    T_GE
    { $$ = $1; } |

    T_EQ
    { $$ = $1; } |

    T_NE
    { $$ = $1; } ;

comp_expr:
    bitor_expr
    { $$ = $1; } |

    comp_expr cmp_op bitor_expr
    { $$ = new ast::Binary($2, $1, $3); } ;

bitor_expr:
    bitxor_expr
    { $$ = $1; } |

    bitor_expr '|' bitxor_expr
    { $$ = new ast::Binary(T_BITOR, $1, $3); } ;

bitxor_expr:
    bitand_expr
    { $$ = $1; } |

    bitxor_expr '^' bitand_expr
    { $$ = new ast::Binary(T_BITXOR, $1, $3); } ;

bitand_expr:
    bitshift_expr
    { $$ = $1; } |

    bitand_expr '&' bitshift_expr
    { $$ = new ast::Binary(T_BITAND, $1, $3); } ;

bitshift_op:
    T_LSHIFT
    { $$ = $1; } |

    T_RSHIFT
    { $$ = $1; } ;

bitshift_expr:
    arith_expr
    { $$ = $1; } |

    bitshift_expr bitshift_op arith_expr
    { $$ = new ast::Binary($2, $1, $3); } ;

arith_op:
    '+'
    { $$ = T_ADD; } |

    '-'
    { $$ = T_SUB; } ;

arith_expr:
    term_expr
    { $$ = $1; } |

    arith_expr arith_op term_expr
    { $$ = new ast::Binary($2, $1, $3); } ;

term_op:
    '*'
    { $$ = T_MUL; } |

    '/'
    { $$ = T_DIV; } |

    '%'
    { $$ = T_MOD; } ;

term_expr:
    power_expr
    { $$ = $1; } |

    term_expr term_op power_expr
    { $$ = new ast::Binary($2, $1, $3); } ;

power_expr:
    sexpr
    { $$ = $1; } |

    power_expr T_POW sexpr
    { $$ = new ast::Binary($2, $1, $3); } ;

sexpr:
    '+' sexpr %prec T_BITNEG
    { $$ = new ast::Unary(T_ADD, $2); } |

    '-' sexpr %prec T_BITNEG
    { $$ = new ast::Unary(T_SUB, $2); } |

    T_BITNEG sexpr
    { $$ = new ast::Unary(T_BITNEG, $2); } |

    '(' expr ')'
    { $$ = $2; } |

    array_expr
    { $$ = $1; } |

    call_expr
    { $$ = $1; } |

    subs_expr
    { $$ = $1; } |

    func_expr
    { $$ = $1; } |

    TRUE
    { $$ = new ast::True(); } |

    FALSE
    { $$ = new ast::False(); } |

    INTEGER
    { $$ = new ast::Integer($1); } |

    REAL
    { $$ = new ast::Real($1); } |

    STRING
    { $$ = new ast::String($1); } |

    IDENTIFIER
    { $$ = new ast::Variable($1); } ;
%%
