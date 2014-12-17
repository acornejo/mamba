%{
#include <vector>
#include <string>
#include <stdlib.h>

std::vector<char> paren;
std::vector<int> indent;
int pending_indents = 0, pending_dedents = 0;

char stre[512];

extern void yyerror(void *, const char *);

#define paren_add(c) paren.push_back(c)
#define paren_del(c)\
if (paren.empty() || paren.back() != c) {\
    sprintf(stre, "SyntaxError: expected '%c' but found '%c'", c, paren.back());\
    yyerror(NULL, stre);\
} else\
    paren.pop_back()
#define TK(t) (yylval->token = t)
/*int TK(int t) {*/
/*    yylval.token = t;*/
/*    if (t == INDENT)*/
/*        printf("parsed token '%s'\n", "INDENT");*/
/*    else if (t == DEDENT)*/
/*        printf("parsed token '%s'\n", "DEDENT");*/
/*    else if (t == NEWLINE)*/
/*        printf("parsed token '%s'\n", "NEWLINE");*/
/*    else*/
/*        printf("parsed token '%s'\n", yytext);*/
/*    return t;*/
/*}*/

%}

%option bison-bridge
%option reentrant
%option noyywrap

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

"->"            { return TK(T_RETURNS); }

"<="            { return TK(T_LE); }
">="            { return TK(T_GE); }
"=="            { return TK(T_EQ); }
"!="            { return TK(T_NE); }

"//"            { return TK(T_IDIV); }
"**"            { return TK(T_POW); }
 /* ".."        { return TK(T_RANGE); } */

"<<"            { return TK(T_LSHIFT); }
">>"            { return TK(T_RSHIFT); }
"&"             { return TK(T_BITAND); }
"^"             { return TK(T_BITXOR); }
"~"             { return TK(T_BITNEG); }

"let"           { return TK(LET); }
"false"         { return TK(FALSE); }
"true"          { return TK(TRUE); }
"record"        { return TK(RECORD); }
"tuple"         { return TK(TUPLE); }
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
