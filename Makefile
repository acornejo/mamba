# CC CXX CCFLAGS CPPFLAGS CXXFLAGS LDFLAGS LDLIBS $< (input) $@ (outout) $^ (dependencies)
EXEC := mamba
SRCS := $(wildcard *.cc)
OBJS := ${SRCS:.cc=.o}
DEPS := ${SRCS:.cc=.d}

CC := g++ -g
CXXFLAGS :=`pkg-config --cflags clutter-1.0`
LDLIBS :=`pkg-config --libs clutter-1.0`
OUTPUT_OPTION=-g -MMD -MP -Wall -o $@
LEX := flex
YACC := bison

all: $(EXEC) info

$(EXEC): $(OBJS)

lexer.cc lexer.h: mamba.l parser.h
	$(LEX) -o lexer.cc --header-file=lexer.h mypy.l

parser.cc parser.h: mamba.y
	$(YACC) -d -o parser.cc mypy.y

.PHONY: clean info

info:
	bison --version |  awk 'NR<1 match($0,/([0-9]+\.?)+/) {print Bison substr($0,RSTART,RLENGTH)}'
	flex --version |  awk 'NR<1 match($0,/([0-9]+\.?)+/) {print Flex substr($0,RSTART,RLENGTH)}'

clean:
	rm -f $(EXEC) $(OBJS) $(DEPS)

-include $(DEPS)
