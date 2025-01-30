CC := gcc
CFLAGS := -g
LDFLAGS := -lX11 -lm
TARGET := main
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.c,%.d,$(SRC))

$(TARGET): $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -MMD -c

.PHONY: clean
clean:
	rm -rf $(OBJ) $(DEP) $(TARGET) image.ppm image.png