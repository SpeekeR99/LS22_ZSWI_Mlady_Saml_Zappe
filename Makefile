# C compiler
CC=gcc

# C compilation output
ifeq ($(OS),Windows_NT) 
	COUT=server.exe
else
	COUT=server
endif

# C compilation flags
CFLAGS=-Wall -lpthread -lm
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
	rm -f DATA/sim_frames/*
	rm -f DATA/vis_frames/*
	rm -f DATA/merged.csv
	rm -f PY/__pycache__/*
	rmdir PY/__pycache__

clean: 
	rm -f DATA/sim_frames/*
	rm -f DATA/vis_frames/*
	rm -f DATA/merged.csv