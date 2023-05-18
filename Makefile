CC = gcc

CFLAGS = -Wall -Wno-unknown-pragmas -Wno-implicit-function-declaration -Wno-unused-variable -g

SRC = src
LIB = lib
INCLUDE = include
BIN = bin

RECEIVER = main/noncanonical.c
TRANSMITTER = main/writenoncanonical.c
BUILDEXTENS = out

SERIAL1 = /dev/ttyS10
SERIAL2 = /dev/ttyS11

# Targets
.PHONY: all

all: $(BIN)/receiver.$(BUILDEXTENS) $(BIN)/transmitter.$(BUILDEXTENS)

$(BIN)/receiver.$(BUILDEXTENS): $(RECEIVER) #$(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE) -lrt
#	$(CC) $(CFLAGS) -o $@ $^ 

$(BIN)/transmitter.$(BUILDEXTENS): $(TRANSMITTER) #$(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE) -lrt
#	$(CC) $(CFLAGS) -o $@ $^ 

.PHONY: clean
clean:
	rm -f $(BIN)/*

.PHONY: socat
socat: 
	sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777

.PHONY: runt
runt:
	./$(BIN)/transmitter.$(BUILDEXTENS) $(SERIAL1)

.PHONY: runr
runr:
	./$(BIN)/receiver.$(BUILDEXTENS) $(SERIAL2)