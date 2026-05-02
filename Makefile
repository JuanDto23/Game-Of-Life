CC = gcc
MPICC = mpicc
CFLAGS = -Wall -I$(INCLUDE_DIR)

BIN_DIR = bin
SRC_DIR = src
INCLUDE_DIR = include

_SRC_FILES_SEQ = gol_seq.c main_seq.c pbm_io.c
SRC_FILES_SEQ = ${addprefix ${SRC_DIR}/, ${_SRC_FILES_SEQ}}

_SRC_FILES_MPI = gol_mpi.c main_mpi.c pbm_io.c
SRC_FILES_MPI = ${addprefix ${SRC_DIR}/, ${_SRC_FILES_MPI}}

TARGETS = seq mpi

all: ${TARGETS}

seq:
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC_FILES_SEQ) -o $(BIN_DIR)/gol_seq && ./$(BIN_DIR)/gol_seq

mpi:
	@mkdir -p $(BIN_DIR)
	$(MPICC) $(CFLAGS) $(SRC_FILES_MPI) -o $(BIN_DIR)/gol_mpi && ./$(BIN_DIR)/gol_mpi

clean:
	rm -rf $(BIN_DIR)/*
	@echo "Executables files were deleted."