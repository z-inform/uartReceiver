BUILDDIR = output
CFLAGS = -Wall -Wpedantic

ULIBTARGETS = circ_buffer.o
FINALOBJ := main.o $(ULIBTARGETS)
FINALOBJ := $(addprefix $(BUILDDIR)/, $(FINALOBJ))

$(BUILDDIR)/receiver.elf: $(FINALOBJ)
	gcc $(FINALOBJ) -o $@

$(BUILDDIR)/main.o : main.c $(addsuffix .h, $(basename $(ULIBATRGETS)))
	gcc -c $(CFLAGS) main.c -o $@

$(addprefix $(BUILDDIR)/, $(ULIBTARGETS)) : $(BUILDDIR)/%.o : %.c %.h
	gcc -c $(CFLAGS) $< -o $@

