CC:=gcc
CFLAGS:=-Wall

INCLUDE_DIR:=./include
BUILD_FLAGS:=-I$(INCLUDE_DIR)

BIN:=esp_over_ubus
SRC:=$(wildcard *.c)

LIBS:=-lserialport -lubus -lubox -lblobmsg_json -ljansson

.PHONY: all clean

all:$(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(BUILD_FLAGS) $(LIBS)
	
clean:
	$(RM) $(BIN)
