TARGET_EXEC := download
BUILD_DIR := ./build
SRC_DIR := ./src

CC := gcc
CFLAGS := -g -Wall -pipe

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS) -c $< -o $@

	
.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)