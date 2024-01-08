# compiler / kompilátor
CC = gcc

# compiler switches / přepínače pro kompilátor
CFLAGS = -Wall -D_REENTRANT
# -Wall		warnings: all / vypisovat všechna varování
# -g		include debugging symbols / zahrnout symboly pro debugger
# -Dsymbol	define symbol like #define / definuje symbol jako #define
# -D_REENTRANT

# linker switches / přepínače pro linker
LDFLAGS =
# link libraries / knihovny pro linker
LDLIBS = -lpthread
# -llibrary / -lknihovna
#  libNAME.so.version	filename of the library / jméno souboru knihovny
# -lNAME
# e.g. / např.: -lpthread	link with libpthread.so / připojit knihovnu libpthread.so

RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROGRAMS = pthread_create pthread_create2 \
	   pthread_detach \
	   hello_threads \
	   pthread_arg_passing_bad pthread_arg_passing_good \
	   pthread_join pthread_join2 pthread_join3 \
	   pthread_result_passing \
	   mem_addresses \
	   mem_addresses32
INDIVIDUALLY = 

all: $(PROGRAMS)

solo: $(INDIVIDUALLY)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

mem: mem_addresses
	./mem_addresses | LANG=C sort -tx -k2r

mem32: mem_addresses32
	./mem_addresses32 | LANG=C sort -tx -k2r

mem_addresses32: CFLAGS += -m32

mem_addresses32: mem_addresses.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LDLIBS)

clean:
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS) $(INDIVIDUALLY)

