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
LDLIBS =
# -llibrary / -lknihovna
#  libNAME.so.version	filename of the library / jméno souboru knihovny
# -lNAME
# e.g. / např.: -lpthread	link with libpthread.so / připojit knihovnu libpthread.so

SHELL = /bin/bash
RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROG_STORE_SV = store_shmSV
PROG_READ_SV = read_shmSV
PROGRAMS_SV = $(PROG_STORE_SV) $(PROG_READ_SV)
PROGRAMS = $(PROGRAMS_SV)

TEXTS = 8
TEXTS_URL = http://metaphorpsum.com/paragraphs/$(TEXTS)/1

all: $(PROGRAMS)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

test: testSV

testSV: $(PROGRAMS_SV)
	@printf -v NL '\n'; \
	IFS="$$NL" read -r -d "" -a ARGS <<< "$$(wget -qO- "$(TEXTS_URL)")"; \
	SHMID="$$(./$(PROG_STORE_SV) 0600 "$${ARGS[@]}")"; \
	: echo "SHM id = $$SHMID"; \
	echo; LANG=C ipcs -m | egrep "key| $$SHMID " || :; echo; \
	./$(PROG_READ_SV) "$$SHMID"; \
	echo; LANG=C ipcs -m | egrep "key| $$SHMID " || :; echo

show:
	LANG=C ipcs -m

clean:
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS)

