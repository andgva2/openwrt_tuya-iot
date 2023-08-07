CC:=gcc
CFLAGS:=-Wall

LUA_INCLUDE_DIR:=/usr/include/lua5.4
LUA_LIB_DIR:=/usr/lib/x86_64-linux-gnu/

SCR_INCLUDE_DIR:=./include
BUILD_FLAGS:=-I$(SCR_INCLUDE_DIR) -I$(LUA_INCLUDE_DIR)

BIN:=tuya_daemon
SRC:=$(wildcard *.c)

LIB_FLAGS:=-L$(LUA_LIB_DIR)
LIBS:=-largp -llua5.4

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(BUILD_FLAGS) $(LIB_FLAGS) $(LIBS)

clean:
	$(RM) $(BIN)
