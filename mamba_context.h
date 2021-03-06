#ifndef MAMBACONTEXT_H__
#define MAMBACONTEXT_H__

#include <iostream>
#include <fstream>
#include "ast.h"

class MambaContext {
    private:
        std::ifstream input;
        ast::Node *output;
        void *scanner;

    public:
        MambaContext();
        virtual ~MambaContext();
        int parse(const char *name=NULL);
        int read(char *buf, int max_size);
        void *getScanner() { return scanner; }
        ast::Node *getOutput() { return output; }
        void setOutput(ast::Node *_output) { output = _output; }
};

#include "parser.h"
#define YY_EXTRA_TYPE MambaContext*
#define YY_NO_INPUT
#define YY_INPUT(buf, result, max_size) result = yyextra->read(buf,max_size)

extern void yyerror(YYLTYPE *, MambaContext *context, const char *);

#endif//MAMBACONTEXT_H__
