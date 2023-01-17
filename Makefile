CC = gcc
LDFLAGS =
BLDDIR = .
INCDIR = $(BLDDIR)/inc
SRCDIR = $(BLDDIR)/src
OBJDIR = $(BLDDIR)/obj
CFLAGS = -c -Wall -I$(INCDIR) -lwiringPi
SRC = $(wildcard $(SRCDIR)/*.c)
INC = $(wildcard $(INCDIR)/*.c, $(OBJDIR)/%.o)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
EXE = bin/bin

ARGS ?= /dev/i2c-1

all: clean $(EXE) 
    
$(EXE): $(OBJ)
	$(CC) $(INCDIR)/*.c $(LDFLAGS) $(OBJDIR)/*.o -o $@ 

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJDIR)/*.o $(EXE)

run:
	$(EXE) $(ARGS)
