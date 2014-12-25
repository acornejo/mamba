#include <iostream>
#include "mamba_context.h"

void yyerror(YYLTYPE *yylloc, MambaContext *context, const char *err) {
    std::cout << err << "\n";
    std::cout << "line: " << yylloc->last_line << "-" <<yylloc->first_line << "\n";
    std::cout << "column: " << yylloc->last_column << "-" <<yylloc->first_column<< "\n";
}

int main(int argc, char *argv[]) {
    MambaContext ctx(std::cin);
    ctx.parse();
    std::cout << ctx.getOutput() << std::endl;
    return 0;
}
