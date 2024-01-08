# compiler
JAVAC = javac

# interpreter
JAVA = java

PROGRAM = ProducerConsumer
SOLUTION = ProducerConsumerMonitor

all: $(PROGRAM)

solution: $(SOLUTION)

%: %.java
	$(JAVAC) $<

test: $(PROGRAM)
	$(JAVA) $<
	$(JAVA) $< 3 2
	$(JAVA) $< 1 20

testmon: $(SOLUTION)
	$(JAVA) $<
	$(JAVA) $< 3 2
	$(JAVA) $< 1 20

clean:
	$(RM) *.class *~
