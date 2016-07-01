%{
#include <vector>
#include <string>
#include "mamba_context.h"

std::vector<char> paren;
std::vector<size_t> indent;
int pending_indents = 0, pending_dedents = 0;

#define paren_add(c) paren.push_back(c)
#define paren_del(c)\
if (paren.empty() || paren.back() != c) {\
    char stre[512];\
    sprintf(stre, "syntax error, unexpected '%c', expected '%c'", paren.back(), c);\
    yyerror(yylloc, yyextra, stre);\
} else\
    paren.pop_back()
#define TK(t) (yylval->token = t)

#define YY_USER_ACTION yylloc->first_line = yylineno;
%}

%option outfile="lexer.cc" header-file="lexer.h"
%option reentrant
%option bison-bridge
%option bison-locations
%option noyywrap
%option yylineno

digit		[0-9]
integer     {digit}+
exponent    [eE][+-]?{integer}
real        ({integer}("."{integer})?|"."{integer}){exponent}?
letter      [a-zA-Z]
identifier  ({letter}|"_")({digit}|{letter}|"_")*
string      L?\"(\\.|[^\\"])*\"

%%
[\t\n ]+\n      { unput('\n'); }

\n[\t ]*        {
                    if (pending_indents > 0) {
                        pending_indents--;
                        if (pending_indents > 0)
                            yyless(0);
                        return TK(INDENT);
                    }
                    if (pending_dedents > 0) {
                        pending_dedents--;
                        if (pending_dedents > 0)
                            yyless(0);
                        return TK(DEDENT);
                    }
                    if (yyleng > 1 && (indent.empty() || indent.back() < yyleng)) {
                        indent.push_back(yyleng);
                        pending_indents++;
                        yyless(0);
                    } else {
                        if (!indent.empty() && indent.back() > yyleng) {
                            while(!indent.empty() && indent.back() > yyleng) {
                                indent.pop_back();
                                pending_dedents++;
                            }
                            yyless(0);
                        }
                    }
                    return TK(NEWLINE);
                }
<<EOF>>         {
                    if (!indent.empty()) {
                        indent.pop_back();
                        return DEDENT;
                    }
                    yyterminate();
                }
[\t ]*          { /* ignore white space */ }
#[^\n]*$        { /* ignore comments */ }

"("             { paren_add(')'); return TK('('); }
")"             { paren_del(')'); return TK(')'); }
"{"             { paren_add('}'); return TK('{'); }
"}"             { paren_del('}'); return TK('}'); }
"["             { paren_add(']'); return TK('['); }
"]"             { paren_del(']'); return TK(']'); }
":"             { return TK(':'); }
"."             { return TK('.'); }
","             { return TK(','); }
";"             { return TK(';'); }
"="             { return TK('='); }
"<"             { return TK('<'); }
">"             { return TK('>'); }
"|"             { return TK('|'); }
"+"             { return TK('+'); }
"-"             { return TK('-'); }
"*"             { return TK('*'); }
"/"             { return TK('/'); }
"%"             { return TK('%'); }

"..."           { return TK(T_ELLIPSIS); }
"->"            { return TK(T_ARROW); }

"<="            { return TK(T_LE); }
">="            { return TK(T_GE); }
"=="            { return TK(T_EQ); }
"!="            { return TK(T_NE); }

"**"            { return TK(T_POW); }

"<<"            { return TK(T_LSHIFT); }
">>"            { return TK(T_RSHIFT); }
"&"             { return TK(T_BITAND); }
"^"             { return TK(T_BITXOR); }
"~"             { return TK(T_BITNEG); }

"let"           { return TK(LET); }
"fun"           { return TK(FUN); }
"False"         { return TK(FALSE); }
"True"          { return TK(TRUE); }
"record"        { return TK(RECORD); }
"union"         { return TK(UNION); }
"or"            { return TK(OR); }
"and"           { return TK(AND); }
"not"           { return TK(NOT); }
"if"            { return TK(IF); }
"else"          { return TK(ELSE); }
"elif"          { return TK(ELIF); }
"while"         { return TK(WHILE); }
"for"           { return TK(FOR); }
"break"         { return TK(BREAK); }
"continue"      { return TK(CONTINUE); }
"return"        { return TK(RETURN); }

{integer}       {
                    yylval->integer = strtol(yytext, NULL, 0);
                    return INTEGER;
                }
{real}          {
                    yylval->real = strtod(yytext, NULL);
                    return REAL;
                }
{string}        {
                    yylval->string = new std::string(yytext, yyleng);
                    return STRING;
                }
{identifier}    {
                    yylval->string = new std::string(yytext, yyleng);
                    return IDENTIFIER;
                }
%%
