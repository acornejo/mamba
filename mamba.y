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
    ast::TypeList *tlist;
    ast::Type *type;
}

%nonassoc '('
%nonassoc '['
%left '.'
%right NOT
%right T_BITNEG
%left OR
%left AND
%left T_LT
%left T_GT
%left T_LE
%left T_GE
%left T_EQ
%left T_NE
%left T_BITOR
%left T_BITAND
%left T_BITXOR
%left T_LSHIFT
%left T_RSHIFT
%left T_ADD
%left T_SUB
%left T_MUL
%left T_DIV
%left T_MOD
%left T_POW

%token<string> IDENTIFIER STRING
%token<real> REAL
%token<integer> INTEGER
%token<token> INDENT DEDENT NEWLINE
%token<token> T_ARROW T_ELLIPSIS
%token<token> LET FUN FALSE TRUE RECORD UNION IF ELSE ELIF WHILE BREAK CONTINUE FOR IN RETURN

%token<token> NOT T_BITNEG OR AND T_LT T_GT T_LE T_GE T_EQ T_NE T_BITOR T_BITAND T_BITXOR T_LSHIFT T_RSHIFT T_ADD T_SUB T_MUL T_DIV T_MOD T_POW

/* Clean up memory in case of error */
%destructor { delete $$; } <node>
%destructor { delete $$; } IDENTIFIER
%destructor { delete $$; } STRING

%type<node> suite stmt_block simple_stmt small_stmt compound_stmt assn_stmt let_stmt break_stmt continue_stmt return_stmt while_stmt for_stmt if_stmt elif_stmt record_suite record_stmt union_decl union_block union_suite union_stmt
%type<node> func_expr expr_list_ne expr_list expr
%type<type> pointer_type array_type ref_type tuple_type func_type type
%type<tlist> record_block func_params type_list

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

    let_stmt
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
    expr '=' expr
    { $$ = new ast::Assign($1, $3); } |

    expr '=' assn_stmt
    { $$ = $3; ((ast::Assign*)$$)->vars.push_back($1); } ;

let_stmt:
    LET IDENTIFIER '=' expr
    { $$ = new ast::VarDecl($2, $4); } |

    LET IDENTIFIER '=' func_expr
    { $$ = new ast::FuncDecl($2, $4); } ;

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

record_suite:
    NEWLINE INDENT record_block DEDENT
    { $$ = $3; } ;

record_block:
    type IDENTIFIER NEWLINE
    { $$ = new ast::TypeList(); $$->appendNamedChild($2, $1); } |

    record_block type IDENTIFIER NEWLINE
    { $$ = $1; $$->appendNamedChild($3, $2); } ;

union_suite:
    NEWLINE INDENT union_block DEDENT
    { $$ = $3; } ;

union_block:
    union_decl NEWLINE
    { $$ = new ast::UnionList(); $$->appendChild($1); } |

    union_block union_decl NEWLINE
    { $$ = $1; $1->appendChild($2); } ;

union_decl:
    IDENTIFIER
    { $$ = new ast::UnionItem($1, NULL); } |

    IDENTIFIER '(' type ')'
    { $$ = new ast::UnionItem($1, $3); } ;

pointer_type:
    '*' type
    { $$ = new ast::PtrType($2); } ;

array_type:
    '[' type ']'
    { $$ = new ast::ArrayType($2); } ;

array_type:
    '[' type ':' type ']'
    { $$ = new ast::MapType($2, $4); } ;

ref_type:
    '&' type
    { $$ = new ast::RefType($2); } ;

type_list:
    type
    { $$ = new ast::TypeList(); $$->appendChild($1); } |

    type_list ',' type
    { $$ = $1; $$->appendChild($3); } ;

tuple_type:
    '(' type_list ')'
    { $$ = new ast::TupleType($2); } ;

func_type:
    '(' ')' T_ARROW type
    { $$ = new ast::FuncType(new ast::TypeList(), $4); } |

    '(' type_list ')' T_ARROW type
    { $$ = new ast::FuncType($2, $5); } ;

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

func_params:
    type IDENTIFIER
    { $$ = new ast::TypeList(); $$->appendNamedChild($2, $1); } |

    func_params ',' type IDENTIFIER
    { $$ = $1; $$->appendNamedChild($4, $3); } ;

func_expr:
    FUN '(' ')' T_ARROW type ':' suite
    { $$ = new ast::Function(new ast::FuncType(new ast::TypeList(), $5), $7); } |

    FUN '(' func_params ')' T_ARROW type ':' suite
    { $$ = new ast::Function(new ast::FuncType($3, $6), $8); } ;

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

expr:
    NOT expr
    { $$ = new ast::Unary($1, $2); } |

    T_BITNEG expr
    { $$ = new ast::Unary($1, $2); } |

    expr AND expr
    { $$ = new ast::And($1, $3); } |

    expr OR expr
    { $$ = new ast::Or($1, $3); } |

    expr T_LT expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_GT expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_LE expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_GE expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_EQ expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_NE expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_BITAND expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_BITOR expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_BITXOR expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_LSHIFT expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_RSHIFT expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_ADD expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_SUB expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_MUL expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_DIV expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_MOD expr
    { $$ = new ast::Binary($2, $1, $3); } |

    expr T_POW expr
    { $$ = new ast::Binary($2, $1, $3); } |

    '(' expr ')'
    { $$ = $2; } |

    '[' expr_list_ne ']'
    { $$ = new ast::Array($2); } |

    expr '(' expr_list ')'
    { $$ = new ast::Call($1, $3); } |

    expr '[' expr ']'
    { $$ = new ast::Subscript($1, $3); } |

    expr '.' IDENTIFIER
    { $$ = new ast::Attribute($1, $3); } |

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
