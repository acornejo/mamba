#include "mamba_context.h"
#include "lexer.h"

MambaContext::MambaContext(std::istream &_input): input(_input), output(NULL) {
    yylex_init(&scanner);
    yyset_extra(this, scanner);
}

MambaContext::~MambaContext() {
    yylex_destroy(scanner);
    if (output)
        delete output;
}

int MambaContext::parse() {
    return yyparse(this);
}

int MambaContext::read(char *buf, int max_size) {
    if (input.eof() || input.fail())
        return 0;

#define YY_INTERACTIVE
#ifdef YY_INTERACTIVE
    input.get(buf[0]);
    if (input.eof())
        return 0;
    else if (input.bad())
        return 0;//-1
    else
        return 1;
#else
    input.read(buf, max_size);
    if (input.bad())
        return 0;//-1
    else
        return input.gcount();
#endif
}
