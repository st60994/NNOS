CC = mcs

PROG = hello-world

EXE = $(PROG:%=%.exe)

all: $(EXE)

%.exe: %.cs
	$(CC) "$<"

test: $(EXE)
	./$(EXE)

clean:
	rm -f *~ *.bak $(EXE)
