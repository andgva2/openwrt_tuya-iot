CC:=gcc
CFLAGS:=-Wall

INCLUDE_DIR:=./include
BUILD_FLAGS:=-I$(INCLUDE_DIR)

BIN:=tuya_daemon
SRC:=$(wildcard *.c)

LIBS:=-llink_core -lplatform_port -lmiddleware_implementation -lutils_modules #-lubus -lubox -lblobmsg_json

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(BUILD_FLAGS) $^ -o $@ $(LIBS)

clean:
	$(RM) $(BIN)
