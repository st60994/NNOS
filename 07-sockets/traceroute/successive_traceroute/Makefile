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

TR = traceroute
OBJEKTY = ip_stuff.o options.o $(TR).o
ZALOHY = *~ *.bak
PROGRAMY = $(TR)

all: $(PROGRAMY)

$(TR): $(OBJEKTY)
	$(CC) $(OBJEKTY) $(LDFLAGS) -o $@ $(LDLIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Mazu objekty, zalohy a programy
	$(RM) $(OBJEKTY) $(ZALOHY) $(TR).o $(PROGRAMY)
