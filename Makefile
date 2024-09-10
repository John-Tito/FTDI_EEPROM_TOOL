CFLAGS:=-g -O2 -std=c99 -Wall -Wextra
CFLAGS+=-L lib/ftd2xx/amd64
CFLAGS+=-I lib/ftd2xx

EEPROM_TOOLS: src/EEPROM_TOOLS.c
	$(CC) $(CFLAGS) $^ -lftd2xx -o bin/$@

clean:
	rm -rf bin/EEPROM_TOOLS.exe

all: EEPROM_TOOLS
