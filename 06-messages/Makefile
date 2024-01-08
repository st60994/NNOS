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

RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROGRAMS = messagesSV_snd messagesSV_rcv messagesSV_snd_arg messagesSV_rcv_cycle
INDIVIDUALLY = mqPOSIX_snd_arg mqPOSIX_rcv_cycle

all: $(PROGRAMS)

solo: $(INDIVIDUALLY)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

test: testSV

testSV: messagesSV_snd_arg messagesSV_rcv_cycle
	@KEY="`./messagesSV_snd_arg "Hello!" "Glad to see you." "How do you do?" "Hmm." "See you." "CU!"`"; \
	sleep 1; \
	echo "Created message queue with the key: $$KEY"; \
	LANG=C ipcs -q; \
	./messagesSV_rcv_cycle $$KEY; \
	LANG=C ipcs -q


show:
	ipcs -q

clean_queues:
	@echo Removing message queues from system.
	ipcs -q | grep ^0 | tr -s ' ' '\t' | cut -f2 | xargs -rl ipcrm -q || true

rmqu: IPC_TYPE = -q
rmsu: IPC_TYPE = -s
rmmu: IPC_TYPE = -m

rmqu: rmu
rmsu: rmu
rmmu: rmu

rmu:
	@LIST="$$(sudo ipcs $(IPC_TYPE) | grep ^0 | tr -s ' ' '\t' | cut -f2,3)"; \
	printf '%s' "$$LIST" | \
	if [ -z "$(RMUSER)" ]; then \
		cat; \
	else \
		grep $$'\t'"$(RMUSER)\>" | \
		cut -f1 | \
		xargs -rl sudo ipcrm $(IPC_TYPE) || true; \
	fi
	

rmq: clean_queues

clean: clean_queues
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS) $(INDIVIDUALLY)

