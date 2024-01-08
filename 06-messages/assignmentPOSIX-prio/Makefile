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

mqPOSIX%: LDLIBS += -lrt

SHELL = /bin/bash

RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
SYSV_PROG_SND = messagesSV_snd_arg
SYSV_PROG_RCV = messagesSV_rcv_cycle
SYSV_PROGS_MULTI = $(SYSV_PROG_SND) $(SYSV_PROG_RCV)
SYSV_PROGS = messagesSV_snd messagesSV_rcv $(SYSV_PROGS_MULTI)
POSIX_PROG_SND = mqPOSIX_snd_prio_arg
POSIX_PROG_RCV = mqPOSIX_rcv_prio_cycle
POSIX_PROGS = $(POSIX_PROG_SND) $(POSIX_PROG_RCV)

all: $(POSIX_PROGS)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

test: testPOSIX

MQ_NAME := $(shell sed -nr '/^\#define.*MQ_NAME.*"([^"]+)".*/s//\1/p' "$(POSIX_PROG_SND).c")

ARGS = \
 5 "Oba známe tyhle sliby, jsou jen vata." \
 3 "Slíbí si a v davu zmizí." \
 7 "tak já letím," \
11 "Rád tě vidím." \
 9 "Dneska těžko pokecáme, čas je mi v patách." \
10 "Jak je?" \
 2 "Příště…" \
 4 "Někdy příště." \
12 "Ahoj!" \
 8 "Ahoj," \
 1 "už si budou cizí." \
 6 "měj se."

testPOSIX: $(POSIX_PROGS)
	@echo "Queue name: $(MQ_NAME)"; \
	./$(POSIX_PROG_SND) $(ARGS) & \
	sleep 1; \
	find /dev/mqueue/ -user "$$LOGNAME" -ls || true; \
	./$(POSIX_PROG_RCV) "$(MQ_NAME)"; \
	find /dev/mqueue/ -user "$$LOGNAME" -ls || true

testSV: $(SYSV_PROGS_MULTI)
	@KEY="`./messagesSV_snd_arg "Hello!" "Glad to see you." "How do you do?" "Hmm." "See you." "CU!"`"; \
	sleep 1; \
	echo "Created message queue with the key: $$KEY"; \
	LANG=C ipcs -q; \
	./messagesSV_rcv_cycle $$KEY; \
	LANG=C ipcs -q


show: showPOSIX

showSV:
	ipcs -q

showPOSIX:
	find /dev/mqueue/ -user "$$LOGNAME" -ls || true

clean_queues_SV:
	@echo Removing message queues from system.
	ipcs -q | grep ^0 | tr -s ' ' '\t' | cut -f2 | xargs -rl ipcrm -q || true

clean_queues_POSIX:
	@find /dev/mqueue/ -user "$$LOGNAME" -print0 | xargs -r0 rm -v

clean_queues: clean_queues_POSIX

rmq: clean_queues

clean: clean_queues
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(POSIX_PROGS)

