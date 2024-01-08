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

SHELL = /bin/bash
RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROG_STORE_POSIX = store_POSIX
PROG_READ_POSIX = read_POSIX
PROGRAMS_POSIX = $(PROG_STORE_POSIX) $(PROG_READ_POSIX)
PROGRAMS = $(PROGRAMS_POSIX)

SHM_PREFIX = shmOS

TEXTS = 8
TEXTS_URL = http://metaphorpsum.com/paragraphs/$(TEXTS)/1
JOKES_URL = https://v2.jokeapi.dev/joke/Programming?format=txt&amount=$(TEXTS)
SEP = –

all: $(PROGRAMS)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

test: testPOSIX

testPOSIX: $(PROGRAMS_POSIX)
	@printf -v NL '\n'; \
	: IFS="$$NL" read -r -d "" -a ARGS '<<<' "$$(wget -qO- "$(TEXTS_URL)")"; \
	IFS="$(SEP)" read -r -d "" -a ARGS <<< "$$(wget --no-check-certificate -qO- "$(JOKES_URL)" | sed -r -e '/^$$/d' -e 's/^-{5,}$$/$(SEP)/' | tr '\n' ' ' | sed -e 's/ $(SEP) */$(SEP)/g')"; \
	./$(PROG_STORE_POSIX) 0600 "$${ARGS[@]}"; \
	echo; LANG=C ls -l /dev/shm/$(SHM_PREFIX)* || :; echo; \
	./$(PROG_READ_POSIX); \
	echo; LANG=C ls -l /dev/shm/$(SHM_PREFIX)* || :; echo

jokes:
	#wget --no-check-certificate -qO- "$(JOKES_URL)" | sed -r -e '/^$$/d' -e 's/^-{5,}$$/$(SEP)/' | tr '\n' ' ' | sed -e 's/ $(SEP) */$(SEP)/g'
	IFS="$(SEP)" read -r -d "" -a ARGS <<< "$$(wget --no-check-certificate -qO- "$(JOKES_URL)" | sed -r -e '/^$$/d' -e 's/^-{5,}$$/$(SEP)/' | tr '\n' ' ' | sed -e 's/ $(SEP) */$(SEP)/g')"; \
	for A in "$${ARGS[@]}"; do printf '––\n%s\n' "$$A"; done

show:
	ls -l /dev/shm/$(SHM_PREFIX)*

clean:
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS)

