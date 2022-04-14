# C compiler
CC=gcc
# C compilation output
COUT=server.exe
# C compilation flags
CFLAGS=-lpthread -lm
# C source files
CFILES=$(wildcard C/server/*.c) $(wildcard C/simulation/*.c)

# python package manager
PyPM=pip


all: $(COUT)

$(COUT): $(CFILES)
	$(CC) $? $(CFLAGS) -o $(COUT)

install_py_dep:
	$(PyPM) install -r requirements.txt

clean_all: 
	rm -f $(COUT)
	rm -f DATA/frames/*
	rm -f DATA/merged.csv

clean: 
	rm -f DATA/frames/*
	rm -f DATA/merged.csv