# compiler
JAVAC = javac

# interpreter
JAVA = java

PROGRAM = Sync
SOLUTION = SyncMonitor

all: $(PROGRAM)

solution: $(SOLUTION)

%: %.java
	$(JAVAC) $<

test: $(PROGRAM)
	$(JAVA) $<
	$(JAVA) $< 300

testmon: $(SOLUTION)
	$(JAVA) $<
	$(JAVA) $< 300

clean:
	$(RM) *.class *~
