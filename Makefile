# project name (generate executable with this name)
TARGET   = chip8

CC       = gcc
# compiling flags here
CFLAGS   = -Wall -O3

LINKER   = gcc
# linking flags here
LFLAGS   = -Wall -lraylib

# change these to proper directories where each file should be
SRCDIR   = src
OBJDIR   = .obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "comiling $< complete"


.PHONY : clean 
clean : 
	@rm -rf $(OBJDIR)/*
	@echo cleaning complete

.PHONY : delete
delete : 
	@rm -rf $(BINDIR)/*
	@echo remved binaries

