CC := gcc
ASAN ?= 0
CFLAGS := -g
LDFLAGS := -lX11 -lm
TARGET := main
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.c,%.d,$(SRC))

ifeq ($(ASAN), 1)
	CFLAGS += -fsanitize=address -static-libasan
endif

$(TARGET): $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -MMD -c

.PHONY: clean
clean:
	rm -rf $(OBJ) $(DEP) $(TARGET) image.ppm image.png