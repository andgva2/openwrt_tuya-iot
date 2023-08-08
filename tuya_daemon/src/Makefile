CC:=gcc
CFLAGS:=-Wall

SCR_INCLUDE_DIR:=./include
BUILD_FLAGS:=-I$(SCR_INCLUDE_DIR)

BIN:=tuya_daemon
SRC:=$(wildcard *.c)

LIBS:=-llua -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules -largp

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(BUILD_FLAGS) $(LIBS)

clean:
	$(RM) $(BIN)
