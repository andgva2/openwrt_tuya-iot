CC:=gcc
CFLAGS:=-Wall

INCLUDE_DIR:=./include
BUILD_FLAGS:=-I$(INCLUDE_DIR)

BIN:=tuya_daemon
SRC:=$(wildcard *.c)

LIBS:=-llink_core -lplatform_port -lmiddleware_implementation -lutils_modules -largp -lubus -lubox -lblobmsg_json

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(BUILD_FLAGS) $(LIBS)

clean:
	$(RM) $(BIN)
