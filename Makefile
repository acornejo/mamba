# CC CXX CCFLAGS CPPFLAGS CXXFLAGS LDFLAGS LDLIBS $< (input) $@ (outout) $^ (dependencies)
EXEC := mamba
SRCS := $(wildcard *.cc)
OBJS := ${SRCS:.cc=.o} lexer.o parser.o
DEPS := ${SRCS:.cc=.d}

CC := g++ -g
OUTPUT_OPTION=-g -MMD -MP -Wall -o $@
LEX := flex
YACC := bison

all: $(EXEC)

$(EXEC): $(OBJS)

lexer.cc lexer.h: mamba.l parser.cc parser.h
	$(LEX) mamba.l

parser.cc parser.h: mamba.y
	$(YACC) mamba.y

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJS) $(DEPS) lexer.cc lexer.h parser.cc parser.h

-include $(DEPS)
