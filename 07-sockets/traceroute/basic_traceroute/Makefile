# kompilator
CC = gcc

# prepinace pro kompilator
CFLAGS = -Wall
# -Wall		warnings: all
# -g		symboly pro debugger
# -Dsymbol	definuje symbol jako #define symbol
# -D_REENTRANT

# prepinace pro linker
LDFLAGS =
# knihovny pro linker
LDLIBS =
# -lknihovna 
#  libJMENO.so.verze
# -lJMENO
# napr. -lpthread

RM = /bin/rm -f

OBJEKTY = *.o
ZALOHY = *~ *.bak
TR = traceroute
PROGRAMY = $(TR)

all: $(PROGRAMY)

$(TR): $(TR).c $(TR).h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean:
	@echo Mazu objekty, zalohy a programy
	$(RM) $(OBJEKTY) $(ZALOHY) $(PROGRAMY) $(SAMOSTATNE)
