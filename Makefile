##
# better structs
#
# @file
# @version 0.1

CC           := clang++
CC_STANDARD  := c++23
RM           := rm -f
RMDIR        := rm -rf
MKDIR        := mkdir
DEBUGGER     := lldb

INCLUDE      := include
SRC          := src
OBJ          := obj
TARGET       := target

SRCS         := $(shell find $(SRC) -name "*.cpp")
OBJS         := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
LIB          := $(TARGET)/lib.a

TESTDIR      := test
TESTSRC      := $(TESTDIR)/src
TESTINCLUDE  := $(TESTDIR)/include
TESTOBJ      := $(TESTDIR)/obj
TESTBIN      := $(TESTDIR)/bin

TESTSRCS     := $(shell find $(TESTSRC) -name "*.c")
TESTOBJS     := $(patsubst $(TESTSRC)/%.cpp, $(TESTOBJ)/%.o, $(TESTSRCS))
TESTS        := $(patsubst $(TESTOBJ)/%.o, $(TESTBIN)/%, $(TESTOBJS))

LDFLAGS      :=
CFLAGS       := -I$(INCLUDE) -std=$(CC_STANDARD) -Wall -Wextra -Ipfr/include
DEBUGFLAGS   := -O0 -ggdb
TESTFLAGS    := -I$(TESTINCLUDE)

define execute
$(1)

endef

.PHONY: clean debug lldb test all
.SECONDARY: $(TESTOBJS)

all: $(LIB)

$(LIB): $(OBJS) | $(TARGET)
	ar -rc $@ $^

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CC) $(CFLAGS) -c $^ -o $@

test: CFLAGS := $(CFLAGS) $(TESTFLAGS)
test: $(LIB) $(TESTS)
	$(foreach x, $(TESTS), $(call execute, ./$(x)))

debug: CFLAGS := $(CFLAGS) $(DEBUGFLAGS)
debug: all $(TESTS)

$(TESTBIN)/%: $(TESTOBJ)/%.o | $(TESTBIN)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(TESTOBJ)/%.o: $(TESTSRC)/%.c | $(TESTOBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(TESTOBJ) $(TESTBIN) $(OBJ) $(TARGET):
	$(MKDIR) $@

clean:
	$(RMDIR) $(TESTOBJ) $(TESTBIN) $(TARGET) $(OBJ)

# end
