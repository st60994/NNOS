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

# for semaphores and POSIX message queues / pro semafory a posixové fronty zpráv
%_sem %_semN %_msgPOSIX %_semPOSIX %_mqPOSIX cpu_% %_CPUtime: LDLIBS += -lrt


RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROGRAMS = race_fork \
	bank_transactions bank_transactions_Peterson \
	thread_add thread_add_mutex thread_add_sem thread_add_named_sem \
	bounded_buffer
# thread_add_Peterson_xchg bank_deposit
# bank_transactions_Peterson2
INDIVIDUALLY = thread_add_debug thread_add_Peterson thread_add_msgSV thread_add_msgPOSIX bounded_buffer_solved producer_consumer bank_withdrawal
TEMPLATES = cpu_time_measuring cpu_time_measuring2 cpu_time_measuring2_arg bank_deposit_CPUtime

all: $(PROGRAMS)

solo: $(INDIVIDUALLY)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

clean:
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS) $(INDIVIDUALLY) $(TEMPLATES)


# for disassembly / debuggning purposes only:

disassemble: thread_add
	objdump -M intel --disassemble=ThreadAdd $< 2> /dev/null

# only count++ disassembly
count: thread_add
	@#objdump -M intel --disassemble=ThreadAdd $< 2> /dev/null | sed -n '/count/,/count/p'
	objdump -M intel --disassemble=ThreadAdd $< 2> /dev/null | sed -n 'H; s/count/&/; T end; N; N; p; q; :end; g; $$p'

gdb: thread_add_debug
	gdb -batch -ex 'disassemble/m ThreadAdd' $< | sed -n '/count++;/,/^$$/p'

thread_add_debug: CFLAGS += -g
thread_add_debug: rm_thread_add thread_add
	@[ -x $@ ] || cp -av $(word 2,$^) $@

rm_thread_add:
	@file thread_add | grep -q debug_info || rm -fv thread_add

