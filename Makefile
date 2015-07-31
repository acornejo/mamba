# CC CXX CCFLAGS CPPFLAGS CXXFLAGS LDFLAGS LDLIBS $< (input) $@ (outout) $^ (dependencies)
EXEC := main
SRCS := $(wildcard *.cc)
OBJS := ${SRCS:.cc=.o} lexer.o parser.o
DEPS := ${SRCS:.cc=.d}

CC := g++ -g
LLVMFLAGS := $(shell llvm-config --cflags)
CPPFLAGS := $(LLVMFLAGS)
OUTPUT_OPTION=-g -MMD -MP -Wall -o $@
LEX := flex
YACC := bison -r all

$(EXEC): $(OBJS)

main.o: parser.cc lexer.cc

parser.cc: mamba.y
	$(YACC) mamba.y

lexer.cc: mamba.l parser.cc
	$(LEX) -d mamba.l

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJS) $(DEPS) lexer.cc lexer.h parser.cc parser.h

-include $(DEPS)
