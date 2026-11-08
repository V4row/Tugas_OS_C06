CC = gcc

SOURCES = main.c banner.c
HEADERS = banner.h

STATIC_BINARY = main_static
DYNAMIC_BINARY = main_dynamic

.PHONY: all static dynamic clean

all: static dynamic

static: $(STATIC_BINARY)

dynamic: $(DYNAMIC_BINARY)

$(STATIC_BINARY): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -static $(SOURCES) -o $@ $(LDFLAGS)

$(DYNAMIC_BINARY): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(STATIC_BINARY) $(DYNAMIC_BINARY)
