# compiler / kompilátor
CC = gcc

# compiler switches / přepínače pro kompilátor
CFLAGS = -Wall -D_REENTRANT
# -O2		optimize
# -Wall		warnings: all / vypisovat všechna varování
# -g		include debugging symbols / zahrnout symboly pro debugger
# -Dsymbol	define symbol like #define / definuje symbol jako #define
# -D_REENTRANT

# linker switches / přepínače pro linker
LDFLAGS =
# link libraries / knihovny pro linker
LDLIBS = -lpthread -lrt
# -llibrary / -lknihovna
#  libNAME.so.version	filename of the library / jméno souboru knihovny
# -lNAME
# e.g. / např.: -lpthread	link with libpthread.so / připojit knihovnu libpthread.so

SHELL = /bin/bash

RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
# ARGS	= -q -w -a3 -f2 -s0 -t200000000 -c2	# for SemSV version, one run takes ~ 105 minutes, 20 runs = 35 hours
# ARGS	= -q -w -a3 -f2 -s0 -t20000000 -c2	# for SemSV version, one run takes ~ 10.5 minutes, 20 runs = 3.5 hours
# ARGS	= -q -w -a3 -f2 -s0 -t4000000 -c2	# for SemSV version, one run takes ~ 2 minutes, 10 runs = 20 minutes
ARGS	= -q -w -a3 -f2 -s0 -t4000000 -c2
#ARGS4	= -q -w -a3 -f2 -s0 -t2000000 -c4	# not usable for Peterson (limited to two threads)

PROGRAM = bank_deposit_CPUtime

DEPENDS = cs_methods.h
OBJS = 

$(PROGRAM): $(PROGRAM).c $(OBJS) $(DEPENDS)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(OBJS) -o $@ $(LDLIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

# DEBUG:


METHODS = 0 0 1 1 2 2 3 3 4 4 5 6 7 8 9 10
YIELD = -y

# minimum executions of one program
MIN_ROUNDS = 3
# maximum executions of one program
MAX_ROUNDS = 20

# in seconds; maximum time for one program; checked only after the completion of $(MIN_ROUNDS) rounds
TIME_LIMIT = 300

RESULT_FILE = bank_deposit_CPUtime.txt

all: $(PROGRAM)

test: $(PROGRAM)
	@echo >&2
	@echo "CFLAGS used:    $(CFLAGS)" >&2
	@echo "Arguments used: $(ARGS)" >&2
	@echo "Rounds:         $(MAX_ROUNDS)" >&2
	@echo "Limit:          $(MIN_ROUNDS) rounds, $(TIME_LIMIT) seconds per program" >&2
	@[ -r "$(RESULT_FILE)" ] && cp -a "$(RESULT_FILE)" "$(RESULT_FILE).bak" || :
	@LANG=C; \
	calc_dev() { \
		local AVG="$$1" VAL SUM MAX COUNT=0; \
		shift; \
		for VAL; do \
			COUNT=$$(( $$COUNT + 1 )); \
			VAL=$$(( $$VAL - $$AVG )); \
			VAL="$${VAL#-}"; \
			SUM=$$(( $$SUM + $$VAL )); \
			[ -z "$$MAX" -o "$${MAX:-0}" -lt "$$VAL" ] && MAX="$$VAL"; \
		done; \
		if [ "$$COUNT" -gt 0 ]; then \
			if [ "$$AVG" = 0 ]; then \
				echo "$$(( $$SUM / $$COUNT ))–$$(( $$MAX ))"; \
			else \
				echo "$$(( $$SUM / $$COUNT * 100 / $$AVG ))%–$$(( $$MAX * 100 / $$AVG ))%"; \
			fi; \
		else \
			echo "0%–0%"; \
		fi; \
	}; \
	get_method() { \
		sed -r -n "/^[[:space:]]*#[[:space:]]*define[[:space:]]+CS_METHOD_([^[:space:]]+)[[:space:]]+$${1:-0}\>/s//\1/p" cs_methods.h; \
	}; \
	STIME="$$(date "+%F %T")"; \
	echo "Start time:     $$STIME" >&2; \
	echo >&2; \
	LAST_METHOD=; \
	for METHOD in $(METHODS); do \
		: echo >&2; \
		[ "$$LAST_METHOD" = "$$METHOD" ] && USE_YIELD="$(YIELD)" || USE_YIELD=; \
		LAST_METHOD="$$METHOD"; \
		METHOD_STR="$$(get_method "$$METHOD")$${USE_YIELD:++yield}"; \
		METHOD_ARGS="-m $$METHOD $$USE_YIELD"; \
		printf "Test method: %2d %-21s " "$$METHOD" "($$METHOD_STR)" >&2; \
		C=0; \
		SUM_R=0; \
		SUM_U=0; \
		SUM_S=0; \
		TIMES_R=; \
		TIMES_U=; \
		TIMES_S=; \
		REAL_C=0; \
		FAIL=0; \
		while [ "$$C" -lt "$(MAX_ROUNDS)" ]; do \
			C="$$(( $$C + 1 ))"; \
			echo -n . >&2; \
			read R U S <<<"$$(./$(PROGRAM) $$METHOD_ARGS $(ARGS) 2>/dev/null | sed -r -n '/The time.*: ([0-9]+) ([0-9]+) ([0-9]+).*/s//\1\t\2\t\3/p')"; \
			if [ -z "$$R" ]; then \
				FAIL=$$(( $$FAIL + 1 )); \
				echo -n "F" >&2; \
				continue; \
			fi; \
			REAL_C="$$(( $$REAL_C + 1 ))"; \
			TIMES_R="$${TIMES_R:+$$TIMES_R }$$R"; \
			TIMES_U="$${TIMES_U:+$$TIMES_U }$$U"; \
			TIMES_S="$${TIMES_S:+$$TIMES_S }$$S"; \
			SUM_R="$$(( $$SUM_R + $$R ))"; \
			SUM_U="$$(( $$SUM_U + $$U ))"; \
			SUM_S="$$(( $$SUM_S + $$S ))"; \
			: "if the minimum number of rounds is reached: check the time limit:"; \
			if [ "$$REAL_C" -ge "$(MIN_ROUNDS)" -a "$$(( $$SUM_R / 1000 ))" -ge "$(TIME_LIMIT)" ]; then \
				echo -n " (timeout $(TIME_LIMIT) s reached: $$(( $$SUM_R / 1000 )) s, round $$REAL_C)" >&2; \
				break; \
			fi; \
		done; \
		[ "$$FAIL" = 0 ] && SUCCESS="SUCCESS" || SUCCESS="FAILED($$FAIL)"; \
		if [ "$$REAL_C" = 0 ]; then \
			echo " FAILED" >&2; \
			echo "$$METHOD_STR: 0 $$SUCCESS"; \
		else \
			TIME_R="$$(( $$SUM_R / $$REAL_C ))"; \
			TIME_U="$$(( $$SUM_U / $$REAL_C ))"; \
			TIME_S="$$(( $$SUM_S / $$REAL_C ))"; \
			TIME_C="$$(( $$TIME_U + $$TIME_S ))"; \
			DEV_R="$$(calc_dev $$TIME_R $$TIMES_R)"; \
			DEV_U="$$(calc_dev $$TIME_U $$TIMES_U)"; \
			DEV_S="$$(calc_dev $$TIME_S $$TIMES_S)"; \
			: echo -n " $$TIME_R $$TIME_C ($$TIME_U $$TIME_S)" >&2; \
			: echo -n ", ±$$DEV_R ±$$DEV_U ±$$DEV_S" >&2; \
			echo -n " $$TIME_R ±$$DEV_R, $$TIME_C = $$TIME_U ±$$DEV_U  +  $$TIME_S ±$$DEV_S" >&2; \
			echo >&2; \
			printf "%-21s %d %d %d %d" "$$METHOD_STR:" "$$TIME_R" "$$TIME_C" "$$TIME_U" "$$TIME_S"; \
			echo -n " (real CPU user system)"; \
			echo -n ", ±$$DEV_R ±$$DEV_U ±$$DEV_S"; \
			echo -n " (±real ±user ±system)"; \
			echo -n ", $$SUCCESS"; \
			echo    ", all $$REAL_C ($$TIMES_R), ($$TIMES_U), ($$TIMES_S)"; \
		fi; \
	done | sort -k2n > "$(RESULT_FILE)"; \
	echo >> "$(RESULT_FILE)"; \
	TTIME="$$(( ( $$(date "+%s") - $$(date -d "$$STIME" "+%s") ) ))"; \
	TTIME_M="$$(( $$TTIME / 60 ))"; \
	TTIME_S="$$(( $$TTIME % 60 ))"; \
	[ "$$TTIME_S" -lt 10 ] && TTIME_S="0$$TTIME_S"; \
	echo "Start time:     $$STIME" >> "$(RESULT_FILE)"; \
	echo "End time:       $$(date "+%F %T")" >> "$(RESULT_FILE)"; \
	echo "Total time:     $$TTIME_M:$$TTIME_S" >> "$(RESULT_FILE)"
	@echo "CFLAGS used:    $(CFLAGS)" >> "$(RESULT_FILE)"
	@echo "Arguments used: $(ARGS)" >> "$(RESULT_FILE)"
	@echo >> "$(RESULT_FILE)"
	@uname -a >> "$(RESULT_FILE)"
	@echo >> "$(RESULT_FILE)"
	@CPUINFO="$$( cat /proc/cpuinfo )"; \
	CPUS="$$(fgrep processor <<<"$$CPUINFO" | wc -l)"; \
	printf "%d CPU(s)\n\n%s\n" "$$CPUS" "$$(sed -n -e '/^$$/{n;h;n;}' -e 'H' -e '$${x;p;}' <<<"$$CPUINFO")" >> "$(RESULT_FILE)"

clean:
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAM)

