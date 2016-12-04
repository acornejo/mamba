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

%token<string> IDENTIFIER STRING
%token<real> REAL
%token<integer> INTEGER
%token<token> INDENT DEDENT NEWLINE
%token<token> T_ARROW T_ELLIPSIS
%token<token> FUN FALSE TRUE RECORD UNION IF ELSE ELIF WHILE BREAK CONTINUE FOR IN RETURN
%token<token> NOT T_BITNEG OR AND T_LT T_GT T_LE T_GE T_EQ T_NE T_BITOR T_BITAND T_BITXOR T_LSHIFT T_RSHIFT T_ADD T_SUB T_MUL T_DIV T_MOD T_POW T_DEFINE

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

/* Clean up memory in case of error */
%destructor { delete $$; } <node>
%destructor { delete $$; } IDENTIFIER
%destructor { delete $$; } STRING

%type<node> block stmt_list simple_stmt small_stmt compound_stmt assn_stmt var_stmt break_stmt continue_stmt return_stmt while_stmt for_stmt if_stmt elif_stmt record_expr func_stmt record_stmt union_item union_params union_expr union_stmt
%type<node> func_expr expr_list_ne expr_list expr
%type<type> pointer_type array_type ref_type tuple_type func_type return_type type
%type<tlist> record_params func_params type_list

%start program

%%
program:
    stmt_list
    { context->setOutput($1); };

block:
    ':' NEWLINE INDENT stmt_list DEDENT
    { $$ = $4; } ;

stmt_list:
    compound_stmt
    { $$ = new ast::StmtList(); $$->appendChild($1); } |

    simple_stmt
    { $$ = new ast::StmtList(); $$->appendChild($1); } |

    stmt_list simple_stmt
    { $$ = $1; $$->appendChild($2); } |

    stmt_list compound_stmt
    { $$ = $1; $$->appendChild($2); } ;

simple_stmt:
    small_stmt NEWLINE
    { $$ = $1; } ;

small_stmt:
    expr
    { $$ = new ast::Expr($1); } |

    var_stmt
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
    expr '=' expr
    { $$ = new ast::Assign($1, $3); } |

    expr '=' assn_stmt
    { $$ = $3; ((ast::Assign*)$$)->vars.push_back($1); } ;

var_stmt:
    IDENTIFIER T_DEFINE expr
    { $$ = new ast::VarDef($1, $3); } ;

func_stmt:
    IDENTIFIER T_DEFINE func_expr
    { $$ = new ast::FuncDef($1, $3); } ;

record_stmt:
    IDENTIFIER T_DEFINE record_expr
    { $$ = new ast::RecordDef($1, $3); } ;

union_stmt:
    IDENTIFIER T_DEFINE union_expr
    { $$ = new ast::UnionDef($1, $3); } ;

if_stmt:
    IF expr block elif_stmt
    { $$ = new ast::IfElse($2, $3, $4); } ;

elif_stmt:
    %empty
    { $$ = NULL; } |

    ELIF expr block elif_stmt
    { $$ = new ast::IfElse($2, $3, $4); } |

    ELSE block
    { $$ = $2; } ;

while_stmt:
    WHILE expr block
    { $$ = new ast::While($2, $3); } ;

for_stmt:
    FOR IDENTIFIER IN expr block
    { $$ = new ast::For($2, $4, $5); } ;

pointer_type:
    '*' type
    { $$ = new ast::PtrType($2); } ;

array_type:
    '[' type ']'
    { $$ = new ast::ArrayType($2); } ;

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

return_type:
    T_ARROW type
    { $$ = $2; } ;

func_type:
    '(' ')' return_type
    { $$ = new ast::FuncType(new ast::TypeList(), $3); } |

    '(' type_list ')' return_type
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

record_params:
    IDENTIFIER type
    { $$ = new ast::TypeList(); $$->appendNamedChild($1, $2); } |

    record_params ',' IDENTIFIER type
    { $$ = $1; $$->appendNamedChild($3, $4); } ;

record_expr:
    RECORD '(' record_params ')'
    { $$ = $3; } ;

union_item:
    IDENTIFIER
    { $$ = new ast::UnionItem($1, NULL); } |

    IDENTIFIER '(' type ')'
    { $$ = new ast::UnionItem($1, $3); } ;

union_params:
    union_item
    { $$ = new ast::UnionList(); $$->appendChild($1); } |

    union_params ',' union_item
    { $$ = $1; $1->appendChild($3); } ;

union_expr:
    UNION '(' union_params ')'
    { $$ = $3; } ;

func_params:
    type IDENTIFIER
    { $$ = new ast::TypeList(); $$->appendNamedChild($2, $1); } |

    func_params ',' type IDENTIFIER
    { $$ = $1; $$->appendNamedChild($4, $3); } ;

func_expr:
    FUN '(' ')' return_type block
    { $$ = new ast::Function(new ast::FuncType(new ast::TypeList(), $4), $5); } |

    FUN '(' func_params ')' return_type block
    { $$ = new ast::Function(new ast::FuncType($3, $5), $6); } ;

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
