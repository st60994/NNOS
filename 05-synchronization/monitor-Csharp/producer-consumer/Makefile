CC = mcs

PROG = ProducerConsumerSimple

EXE = $(PROG:%=%.exe)

all: $(EXE)

%.exe: %.cs
	$(CC) "$<"

test: $(EXE)
	./$<

clean:
	rm -f *~ *.bak $(EXE)
