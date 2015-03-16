# CC CXX CCFLAGS CPPFLAGS CXXFLAGS LDFLAGS LDLIBS $< (input) $@ (outout) $^ (dependencies)
EXEC := main
SRCS := $(wildcard *.cc)
OBJS := ${SRCS:.cc=.o} lexer.o parser.o
DEPS := ${SRCS:.cc=.d}

CC := g++ -g
LLVMFLAGS := -I/usr/local/Cellar/llvm/3.5.0/include -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
CPPFLAGS := $(LLVMFLAGS)
OUTPUT_OPTION=-g -MMD -MP -Wall -o $@
LEX := flex
YACC := bison

$(EXEC): $(OBJS)

main.o: parser.h lexer.h

parser.cc parser.h: mamba.y
	$(YACC) mamba.y

lexer.cc lexer.h: mamba.l parser.h
	$(LEX) -d mamba.l

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJS) $(DEPS) lexer.cc lexer.h parser.cc parser.h

-include $(DEPS)
