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
	@# fix flex error to avoid warnings
	sed -i -e's/int yyl;/size_t yyl;/g' lexer.cc

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJS) $(DEPS) lexer.cc lexer.h parser.cc parser.h parser.output

-include $(DEPS)
