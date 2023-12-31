NAME = HeliosXEmulator

INC_DIR += ./include
BUILD_DIR ?= ./build


OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)

PLATFORM	?= MACOS


.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
INCLUDES  = $(addprefix -I, $(INC_DIR))
# CFLAGS   += -O2 -MMD -Wall -Werror -ggdb3 $(INCLUDES)
# CFLAGS   += -O2 -MMD -ggdb3 $(INCLUDES)
CFLAGS   += -O2 -fPIC -MMD -ggdb3 $(INCLUDES)

# Files to be compiled
SRCS = $(shell find src/ -name "*.c" | grep -v "src/main.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<


# Depencies
-include $(OBJS:.o=.d)

# Some convenient rules

.PHONY: app run gdb clean run-env
app: $(BINARY)

IMG :=
HEMU_EXEC := $(BINARY) $(ARGS) $(IMG)

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^

run-env: $(BINARY)

run: run-env
	$(HEMU_EXEC)

gdb: run-env
	gdb -s $(BINARY) --args $(HEMU_EXEC)

$(BINARY).so: $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -rdynamic -shared -fPIC -o $@ $^
	@cp $(BINARY).so $(BUILD_DIR)/lib$(NAME).so

$(BINARY).a: $(OBJS)
	@echo + AR $@
	@ar rcs $@ $^
	@mv $(BINARY).a $(BUILD_DIR)/lib$(NAME).a

shared: $(BINARY).so

static: $(BINARY).a

clean:
	-rm -rf $(BUILD_DIR)
