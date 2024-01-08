CC = mcs

PROGS = Transactions TransactionsMonitor

EXES = $(PROGS:%=%.exe)

all: $(EXES)

source: removeMonitor

# remove monitor from the source
removeMonitor: Transactions.cs

Transactions.cs: TransactionsMonitor.cs
	sed -e '/thisLock/s:^://:' -e 's/TransactionsMonitor/Transactions/' "$<" > "$@"


%.exe: %.cs
	$(CC) "$<"

test: Transactions.exe
	./$<

testMon: TransactionsMonitor.exe
	./$<

clean:
	rm -f *~ *.bak $(EXES)
