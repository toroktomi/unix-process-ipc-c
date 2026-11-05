CC      ?= gcc
CFLAGS  ?= -Wall -g

A1 := 01-vineyard-registry
A2 := 02-vineyard-processes-signals-pipes

.PHONY: all run1 run2 clean

all: $(A1)/hegybiro $(A2)/hegybiro

$(A1)/hegybiro: $(A1)/hegybiro.c
	$(CC) $(CFLAGS) -o $@ $<

$(A2)/hegybiro: $(A2)/hegybiro.c
	$(CC) $(CFLAGS) -o $@ $<

# The programs read and write szoleszet.txt in the current directory,
# so they are started from their own folder.
run1: $(A1)/hegybiro
	cd $(A1) && ./hegybiro

run2: $(A2)/hegybiro
	cd $(A2) && ./hegybiro

clean:
	rm -f $(A1)/hegybiro $(A2)/hegybiro $(A1)/temp.txt $(A2)/temp.txt
