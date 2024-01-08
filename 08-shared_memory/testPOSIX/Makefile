# compiler / kompilátor
CC = gcc

# compiler switches / přepínače pro kompilátor
CFLAGS = -Wall
# -Wall		warnings: all / vypisovat všechna varování
# -g		include debugging symbols / zahrnout symboly pro debugger
# -Dsymbol	define symbol like #define / definuje symbol jako #define
# -D_REENTRANT

# linker switches / přepínače pro linker
LDFLAGS =
# link libraries / knihovny pro linker
LDLIBS = -lrt
# -llibrary / -lknihovna
#  libNAME.so.version	filename of the library / jméno souboru knihovny
# -lNAME
# e.g. / např.: -lpthread	link with libpthread.so / připojit knihovnu libpthread.so

RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROGRAM = shm_POSIX_test
INDIVIDUALLY = 
TEST_RUN = ./$(PROGRAM)

all: $(PROGRAM)

solo: $(INDIVIDUALLY)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

test: $(PROGRAM)
	@echo; echo "CREATE AND WRITE"
	$(TEST_RUN) 0
	@echo; echo "CREATE AND WRITE 987"
	$(TEST_RUN) 0 987 || true
	@echo; echo "READ"
	$(TEST_RUN) 1
	@echo; echo "REMOVE"
	$(TEST_RUN) 2
	@echo; echo "READ"
	$(TEST_RUN) 1 || true
	@echo; echo "REMOVE"
	$(TEST_RUN) 2 || true

show:
	ls -l /dev/shm/

clean_shm:
	@echo Removing POSIX shared memory segments from system.
	rm -f /dev/shm/* || true

rmm: clean_shm

clean: clean_shm
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAM) $(INDIVIDUALLY)

